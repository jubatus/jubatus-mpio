// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include <jubatus/mp/wavy.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

bool timer_handler(int* count, mp::wavy::loop* lo)
{
	std::cout << "timer" << std::endl;

	if(++(*count) >= 3) {
		lo->end();
		return false;
	}

	return true;
}

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

int main(void)
{
	mp::wavy::loop lo;

	int count = 0;
	lo.add_timer(0.1, 0.1, timer_handler_binder(&count, &lo));

	lo.run(4);
}

