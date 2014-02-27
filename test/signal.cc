// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include <jubatus/mp/wavy.h>
#include <jubatus/mp/signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

bool signal_handler(int* count, mp::wavy::loop* lo)
{
	std::cout << "signal" << std::endl;

	if(++(*count) >= 3) {
		lo->end();
		return false;
	}

	return true;
}

namespace {
class signal_handler_binder {
public:
	signal_handler_binder(int* count, mp::wavy::loop* lo) :
		m_count(count),
		m_lo(lo)
	{ }

	bool operator()()
	{
		return signal_handler(m_count, m_lo);
	}

private:
	int* m_count;
	mp::wavy::loop* m_lo;
};
}

int main(void)
{
	mp::wavy::loop lo;

	int count = 0;

	// add signal handler before starting any other threads.
	lo.add_signal(SIGUSR1, signal_handler_binder(&count, &lo));

	lo.start(3);

	pid_t pid = getpid();

	usleep(50*1e3);
	kill(pid, SIGUSR1);
	usleep(50*1e3);
	kill(pid, SIGUSR1);
	usleep(50*1e3);
	kill(pid, SIGUSR1);

	lo.join();
}

