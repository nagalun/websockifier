#include "FdWatcher.hpp"

FdWatcher::FdWatcher(uS::Loop * loop, void * data, int fd)
: Poll(loop, fd),
  loop(loop),
  data(data) { }

void FdWatcher::start(int events, void (*callback)(void *, FdWatcher *, int status, int events)) {
	cb = callback;
	Poll::setCb([] (Poll * p, int s, int e) {
		FdWatcher * cs = static_cast<FdWatcher *>(p);
		cs->cb(cs->data, cs, s, e);
	});

	Poll::start(loop, this, events);
}

void FdWatcher::change(int events) {
	Poll::change(loop, this, events);
}

void FdWatcher::close() {
	Poll::stop(loop);
	Poll::close(loop, [] (Poll * p) {
		delete static_cast<FdWatcher *>(p);
	});
}

void FdWatcher::setCb(void (*callback)(void *, FdWatcher *, int status, int events)) {
	cb = callback;
}
