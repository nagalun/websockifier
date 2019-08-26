#include <iostream>
#include <csignal>
#include <string>
#include <memory>
#include <new>

#include <Server.hpp>

/* Just for the signal handler */
std::unique_ptr<Server> s;

void stopServer() {
	s->stop();
}

void outOfMemoryHandler() {
	std::cerr << "Out of mem! Trying to free some." << std::endl;

	if (!s->freeMemory()) {
		std::cerr << "Couldn't free any memory :(" << std::endl;
		throw std::bad_alloc();
	}
}


void signalHandler(int s) {
	stopServer();
}

bool installSignalHandler() {
	return std::signal(SIGINT, signalHandler) != SIG_ERR
		&& std::signal(SIGTERM, signalHandler) != SIG_ERR;
}

int main(int argc, char * argv[]) {
	std::cout << "Starting server..." << std::endl;

	std::set_new_handler(outOfMemoryHandler);

	if (!installSignalHandler()) {
		std::cerr << "Failed to install signal handler" << std::endl;
	}

	s = std::make_unique<Server>(); // TODO: configurable baseDir

	if (!s->listenAndRun(argc >= 2 ? argv[1] : "", argc >= 3 ? std::stoul(argv[2]) : 0)) {
		return 1;
	}

	std::cout << "Server stopped running, exiting." << std::endl;
	return 0;
}
