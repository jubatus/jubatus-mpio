# Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

lib_LTLIBRARIES = libjubatus_mpio.la

libjubatus_mpio_la_LDFLAGS = -version-info 4:2:4
libjubatus_mpio_la_CPPFLAGS = -I..

libjubatus_mpio_la_SOURCES = \
		wavy_connect.cc \
		wavy_listen.cc \
		wavy_loop.cc \
		wavy_signal.cc \
		wavy_timer.cc

noinst_HEADERS = \
		pp.h \
		wavy_kernel.h \
		wavy_kernel_epoll.h \
		wavy_kernel_kqueue.h \
		wavy_loop.h \
		wavy_out.h \
		wavy_out.cc \
		wavy_signal.h \
		wavy_timer.h

