// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include <jubatus/mp/wavy.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

bool timer_handler(int* count, mp::wavy::loop* lo);

namespace {
class timer_handler_binder {
public:
	timer_handler_binder(int* count, mp::wavy::loop* lo) :
		m_count(count),
		m_lo(lo)
	{ }

	bool operator()()
	{
		return timer_handler(m_count, m_lo);
	}

private:
	int* m_count;
	mp::wavy::loop* m_lo;
};
}

bool timer_handler(int* count, mp::wavy::loop* lo)
{
	try {
		std::cout << "*" << std::flush;
		if(--(*count) < 0) {
			lo->end();
		}
		else {
			lo->add_timer(0.001, 0, timer_handler_binder(count, lo));
		}
		return false;
	}
	catch (std::exception& e) {
		std::cerr <<  e.what() << std::endl;
		std::exit(1);
	}
}

int main(void)
{
	mp::wavy::loop lo;
	int count = 2000;
	lo.add_timer(0.01, 0, timer_handler_binder(&count, &lo));
	lo.run(4);
	std::cout << std::endl;
}
