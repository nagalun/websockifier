#pragma once

#include <string>
#include <chrono>
#include <memory>

#include <uWS.h>

#include "explints.hpp"

class FdWatcher;

class Server {
	const std::chrono::steady_clock::time_point startupTime;
	uWS::Hub h;
	// To stop the server from a signal handler, or other thread
	std::unique_ptr<uS::Async, void (*)(uS::Async *)> stopCaller;

	FdWatcher * fd;

public:
	Server(std::string basePath = ".");

	void broadcast(const std::string&);
	bool listenAndRun(std::string addr, u16 port);
	bool freeMemory();
	void stop();

private:
	static void doStop(uS::Async *);
	void unsafeStop();
};
