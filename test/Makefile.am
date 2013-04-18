# Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

AM_CPPFLAGS   = -I..
AM_C_CPPFLAGS = -I..
AM_LDFLAGS = ../mpsrc/libjubatus_mpio.la

check_PROGRAMS = \
		listen_connect \
		handler \
		signal \
		timer \
		many_timer \
		sync \
		huge_message

TESTS = $(check_PROGRAMS)

listen_connect_SOURCES = listen_connect.cc

handler_SOURCES = handler.cc

signal_SOURCES = signal.cc

timer_SOURCES = timer.cc

many_timer_SOURCES = many_timer.cc

sync_SOURCES = sync.cc

huge_message_SOURCES = huge_message.cc
