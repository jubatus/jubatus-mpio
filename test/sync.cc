// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include <jubatus/mp/sync.h>
#include <jubatus/mp/pthread.h>
#include <vector>

struct test {
	test(int num1, int num2) :
		num1(num1), num2(num2) { }

	volatile int num1;
	volatile int num2;
};

void thread_main(mp::sync<test>* obj)
{
	for(int i=0; i < 20; ++i) {
		mp::sync<test>::ref ref(*obj);
		ref->num1++;
		ref->num1--;
		std::cout << (ref->num1 + ref->num2);
	}
}

namespace {
class thread_main_binder {
public:
	explicit thread_main_binder(mp::sync<test>* obj) :
		m_obj(obj)
	{ }

	void operator()()
	{
		thread_main(m_obj);
	}

private:
	mp::sync<test>* m_obj;
};
}

int main(void)
{
	mp::sync<test> obj(0, 0);

	std::vector<mp::pthread_thread> threads(4);
	for(int i=0; i < 4; ++i) {
		threads[i].run(thread_main_binder(&obj));
	}

	for(int i=0; i < 4; ++i) {
		threads[i].join();
	}

	std::cout << std::endl;
}

