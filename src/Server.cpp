#include "Server.hpp"

#include <iostream>
#include <utility>
#include <exception>
#include <new>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

#include "FdWatcher.hpp"

constexpr auto asyncDeleter = [] (uS::Async * a) {
	a->close();
};

Server::Server(std::string basePath)
: startupTime(std::chrono::steady_clock::now()),
  h(uWS::NO_OPTIONS, true, 64),
  stopCaller(new uS::Async(h.getLoop()), asyncDeleter),
  fd(new FdWatcher(h.getLoop(), this, 0)) {
	stopCaller->setData(this);

	h.onHttpRequest([](uWS::HttpResponse * res, uWS::HttpRequest req, char * data, sz_t len, sz_t rem) {
		static const char * web =
			"<!DOCTYPE html>"
			"<html>"
			"<head>"
				"<script>"
					"w=new WebSocket(location.href.replace('http', 'ws'));"
					"w.onmessage = m => document.body.append(m.data, document.createElement('br'));"
					"w.onclose = m => document.body.append('-- CONNECTION CLOSED');"
				"</script>"
			"</head>"
			"<body style=\"font-family: monospace;\"></body>"
			"</html>";
		res->end(web, strlen(web));
	});

	h.getDefaultGroup<uWS::SERVER>().startAutoPing(40000);
}

bool Server::listenAndRun(std::string addr, u16 port) {
	const char * host = addr.size() > 0 ? addr.c_str() : nullptr;

	mode_t m = umask(0);
	if (!h.listen(host, port, nullptr, uS::ListenOptions::ONLY_IPV4)) {
		unsafeStop();
		std::cerr << "Couldn't listen on " << addr << ":" << port << "!" << std::endl;
		return false;
	}
	umask(m);

	std::cout << "Listening on " << addr << ":" << port << std::endl;

	stopCaller->start(Server::doStop);

	fd->start(UV_READABLE, +[] (void * data, FdWatcher * fd, int status, int events) {
		Server &s = *static_cast<Server *>(data);
		std::string line;
		if (!std::getline(std::cin, line)) {
			s.stop();
			//fd->close();
		} else {
			s.broadcast(line);
		}
	});

	try {
		h.run();
	} catch (const std::exception& e) {
		std::cerr << "!!! Uncaught exception occurred! Stopping server." << std::endl;
		std::cerr << "Type: " << typeid(e).name() << std::endl;
		std::cerr << "what(): " << e.what() << std::endl;
		unsafeStop();
		return false;
	}

	return true;
}

void Server::broadcast(const std::string& s) {
	h.getDefaultGroup<uWS::SERVER>().broadcast(s.c_str(), s.size(), uWS::TEXT);
}

bool Server::freeMemory() {
	return false;
}

void Server::unsafeStop() {
	if (stopCaller) {
		h.getDefaultGroup<uWS::SERVER>().close(1012);
		stopCaller = nullptr;
		if (!fd->isClosed()) {
			fd->close();
		}
	}
}

void Server::doStop(uS::Async * a) {
	std::cout << "Signal received, stopping server..." << std::endl;
	Server * s = static_cast<Server *>(a->getData());
	s->unsafeStop();
}

void Server::stop() {
	static sz_t callCount = 0;
	if (stopCaller) {
		stopCaller->send();
	}
	if (++callCount >= 4) {
		std::cerr << "Forcing exit!" << std::endl;
		exit(0);
	}
}
