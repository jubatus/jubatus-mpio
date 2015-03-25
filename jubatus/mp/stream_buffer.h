//
// mpio stream_buffer
//
// Copyright (C) 2008-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MP_STREAM_BUFFER_H__
#define MP_STREAM_BUFFER_H__

#include <memory>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#ifndef MP_STREAM_BUFFER_INITIAL_BUFFER_SIZE
#define MP_STREAM_BUFFER_INITIAL_BUFFER_SIZE 8*1024
#endif

namespace mp {


/**
 * stream_buffer:
 * +-------------------------------------------+
 * | referenced | unparsed data | unused space |
 * +-------------------------------------------+
 * ^            ^ data()        ^ buffer()
 * |
 * |            +---------------+
 * |               data_size()
 * |                            +--------------+
 * |                            buffer_capacity()
 * |
 * |            +-> data_consumed()
 * |
 * |                            +-> buffer_filled()
 * |
 * |                          reserve_buffer() +->
 * |
 * +-- stream_buffer::ref (reference counter)
 *
 */

class stream_buffer {
public:
	explicit stream_buffer(size_t initial_buffer_size = MP_STREAM_BUFFER_INITIAL_BUFFER_SIZE);
	~stream_buffer();

public:
	void reserve_buffer(size_t len,
			size_t initial_buffer_size = MP_STREAM_BUFFER_INITIAL_BUFFER_SIZE);

	void* buffer();
	size_t buffer_capacity() const;
	void buffer_filled(size_t len);

	void* data();
	size_t data_size() const;
	void data_consumed(size_t len);

	class ref {
	public:
		ref();
		explicit ref(const ref& o);
		~ref();
		void clear();
		void swap(ref& x);
	private:
		std::vector<void*> m_array;
		struct each_incr;
		struct each_decr;
	private:
		void push(void* d);
		void move(void* d);
		friend class stream_buffer;
	};

	ref retain();
	void retain_to(ref* to);

private:
	char* m_buffer;
	size_t m_used;
	size_t m_free;
	size_t m_off;
	ref m_ref;

private:
	void expand_buffer(size_t len, size_t initial_buffer_size);

	typedef volatile unsigned int count_t;
	static void init_count(void* d);
	static void decr_count(void* d);
	static void incr_count(void* d);
	static count_t get_count(void* d);

private:
	stream_buffer(const stream_buffer&);
};



inline void stream_buffer::init_count(void* d)
{
	*(volatile count_t*)d = 1;
}

inline void stream_buffer::decr_count(void* d)
{
	//if(--*(count_t*)d == 0) {
	if(__sync_sub_and_fetch((count_t*)d, 1) == 0) {
		free(d);
	}
}

inline void stream_buffer::incr_count(void* d)
{
	//++*(count_t*)d;
	__sync_add_and_fetch((count_t*)d, 1);
}

inline stream_buffer::count_t stream_buffer::get_count(void* d)
{
	return *(count_t*)d;
}


struct stream_buffer::ref::each_incr {
	void operator() (void* d)
	{
		stream_buffer::incr_count(d);
	}
};

struct stream_buffer::ref::each_decr {
	void operator() (void* d)
	{
		stream_buffer::decr_count(d);
	}
};

inline stream_buffer::ref::ref() { }

inline stream_buffer::ref::ref(const ref& o) :
	m_array(m_array)
{
	std::for_each(m_array.begin(), m_array.end(), each_incr());
}

inline void stream_buffer::ref::clear()
{
	std::for_each(m_array.begin(), m_array.end(), each_decr());
	m_array.clear();
}

inline stream_buffer::ref::~ref()
{
	clear();
}

inline void stream_buffer::ref::push(void* d)
{
	m_array.push_back(d);
	incr_count(d);
}

inline void stream_buffer::ref::move(void* d)
{
	m_array.push_back(d);
}

inline void stream_buffer::ref::swap(ref& x)
{
	m_array.swap(x.m_array);
}


inline stream_buffer::stream_buffer(size_t initial_buffer_size) :
	m_buffer(NULL),
	m_used(0),
	m_free(0),
	m_off(0)
{
	const size_t initsz = std::max(initial_buffer_size, sizeof(count_t));

	m_buffer = (char*)::malloc(initsz);
	if(!m_buffer) { throw std::bad_alloc(); }
	init_count(m_buffer);

	m_used = sizeof(count_t);
	m_free = initsz - m_used;
	m_off  = sizeof(count_t);
}

inline stream_buffer::~stream_buffer()
{
	decr_count(m_buffer);
}

inline void* stream_buffer::buffer()
{
	return m_buffer + m_used;
}

inline size_t stream_buffer::buffer_capacity() const
{
	return m_free;
}

inline void stream_buffer::buffer_filled(size_t len)
{
	m_used += len;
	m_free -= len;
}

inline void* stream_buffer::data()
{
	return m_buffer + m_off;
}

inline size_t stream_buffer::data_size() const
{
	return m_used - m_off;
}

inline void stream_buffer::data_consumed(size_t len)
{
	m_off += len;
}


inline stream_buffer::ref stream_buffer::retain()
{
	ref r;
	r.swap(m_ref);
	return r;
}

inline void stream_buffer::retain_to(ref* to)
{
	m_ref.push(m_buffer);
	to->swap(m_ref);
	m_ref.clear();
}

inline void stream_buffer::reserve_buffer(size_t len, size_t initial_buffer_size)
{
	if(m_used == m_off && get_count(m_buffer) == 1) {
		// rewind buffer
		m_free += m_used - sizeof(count_t);
		m_used = sizeof(count_t);
		m_off = sizeof(count_t);
	}
	if(m_free < len) {
		expand_buffer(len, initial_buffer_size);
	}
}

inline void stream_buffer::expand_buffer(size_t len, size_t initial_buffer_size)
{
	if(m_off == sizeof(count_t) && get_count(m_buffer) == 1) {
		size_t next_size = (m_used + m_free) * 2;
		while(next_size < len + m_used) { next_size *= 2; }

		char* tmp = (char*)::realloc(m_buffer, next_size);
		if(!tmp) { throw std::bad_alloc(); }

		m_buffer = tmp;
		m_free = next_size - m_used;

	} else {
		const size_t initsz = std::max(initial_buffer_size, sizeof(count_t));

		size_t next_size = initsz;  // include sizeof(count_t)
		size_t not_used = m_used - m_off;
		while(next_size < len + not_used + sizeof(count_t)) { next_size *= 2; }

		char* tmp = (char*)::malloc(next_size);
		if(!tmp) { throw std::bad_alloc(); }
		init_count(tmp);

		try {
			m_ref.move(m_buffer);
		} catch (...) { free(tmp); throw; }

		memcpy(tmp+sizeof(count_t), m_buffer+m_off, not_used);

		m_buffer = tmp;
		m_used = not_used + sizeof(count_t);
		m_free = next_size - m_used;
		m_off = sizeof(count_t);
	}
}


}  // namespace mp

#endif /* mp/stream_buffer.h */

