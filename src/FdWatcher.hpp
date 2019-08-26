#pragma once

#include <uWS.h>

class FdWatcher : public uS::Poll {
	uS::Loop * loop;
	void * data;
	void (*cb)(void *, FdWatcher *, int, int);

public:
	FdWatcher(uS::Loop * loop, void * data, int fd);

	void start(int events, void (*callback)(void *, FdWatcher *, int status, int events));
	void change(int events);
	void close();
	void setCb(void (*callback)(void *, FdWatcher *, int status, int events));
};
