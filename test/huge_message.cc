// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <jubatus/mp/wavy.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

size_t total_write_size = 1024*1024*128; // KByte

typedef std::vector<char> buffer_t;

class server_handler: public mp::wavy::handler {
public:
	server_handler(int fd, mp::wavy::loop *lo) :
		mp::wavy::handler(fd),
		loop_(lo) {}

	void on_read(mp::wavy::event& ev) {
		buffer_t rbuf(1024);
		ssize_t read_size = ::read(fd(), &rbuf[0], rbuf.size());
		if ( read_size < 0 ) {
			perror("read error");
			exit(1);
		} else if ( read_size == 0 ) {
			ev.remove();
			return;
		}

		std::auto_ptr<buffer_t> buf( new buffer_t(total_write_size) );
		loop_->write(fd(), &((*buf)[0]), buf->size(), buf);
		std::cerr << "wrote " << total_write_size << "bytes"
			  << std::endl;
	}
	static void accepted(mp::wavy::loop *lo, int fd, int err) {
		if(fd < 0) {
			errno = err;
			perror("accept error");
			exit(1);
		}

		try {
			std::cerr << "accepted. " << std::endl;
			lo->add_handler<server_handler>(fd, lo);

		} catch(...) {
			throw;
		}
	}

private:
	mp::wavy::loop *loop_;
};

class client_handler: public mp::wavy::handler {
	class on_timed_out_binder {
	public:
		explicit on_timed_out_binder(mp::wavy::loop* lo) :
			m_lo(lo)
		{ }

		bool operator()()
		{
			return on_timed_out(m_lo);
		}

	private:
		mp::wavy::loop* m_lo;
	};

public:
	client_handler(int fd, mp::wavy::loop *lo) :
		mp::wavy::handler(fd),
		loop_(lo),
		buf_(1024*1024),
		total_size_(0) { }

	void on_read(mp::wavy::event& ev) {
		ssize_t read_size = read( fd(), &buf_[0], buf_.size());
		std::cerr << "." << std::flush;

		if ( read_size <= 0 ) {
			std::cerr << std::endl
				 << "session closed with " << read_size
				 << std::endl;

			ev.remove();
			loop_->end();
			exit(1);
		}
		total_size_ += read_size;

		if ( total_size_ == total_write_size ) {
			std::cerr << std::endl << "OK" << std::endl;
			ev.remove();
			loop_->end();
			return;
		}
	}

	static void connected(mp::wavy::loop *lo, int fd, int err) {
		if ( fd < 0 ) {
			errno = err;
			perror("connect error");
			exit(1);
		}

		try {
			std::cerr << "connected" << std::endl;

			buffer_t buf(32);
			if ( ::write(fd, &buf[0], buf.size()) < 0 ) {
				perror("write error");
				exit(1);
			}

			lo->add_handler<client_handler>(fd, lo);
			lo->add_timer( 10.0, 0.0,
					on_timed_out_binder(lo));
		} catch(...) {
			::close(fd);
			throw;
		}
	}

	static bool on_timed_out(mp::wavy::loop *lo) {
		lo->end();
		std::cerr << "timeout" << std::endl;
		exit(1);
		return false;
	}

private:
	mp::wavy::loop *loop_;
	std::vector<char> buf_;
	size_t total_size_;
};

namespace {
class accepted_binder {
public:
	explicit accepted_binder(mp::wavy::loop* lo_server) :
		m_lo_server(lo_server)
	{ }

	void operator()(int fd, int err) {
		server_handler::accepted(m_lo_server, fd, err);
	}

private:
	mp::wavy::loop* m_lo_server;
};

class connected_binder {
public:
	explicit connected_binder(mp::wavy::loop* lo_client) :
		m_lo_client(lo_client)
	{ }

	void operator()(int fd, int err)
	{
		client_handler::connected(m_lo_client, fd, err);
	}

private:
	mp::wavy::loop* m_lo_client;
};
}

int main(int argc, char **argv)
{
	std::string host = "127.0.0.1";
	int port = 9090;
	if ( argv[1] ) {
		host = argv[1];
		if ( argv[2] ) port = strtoul( argv[2], NULL, 10);
	}

	mp::wavy::loop lo_server;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	lo_server.listen(PF_INET, SOCK_STREAM, 0,
			(struct sockaddr*)&addr, sizeof(addr),
			accepted_binder(&lo_server));

	lo_server.start(1);  // run with 1 threads

	mp::wavy::loop lo_client;
	lo_client.start(2);  // run with 2 threads

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	lo_client.connect(PF_INET, SOCK_STREAM, 0,
			(struct sockaddr*)&addr, sizeof(addr),
			0.0,
			connected_binder(&lo_client));

	lo_client.join();
	lo_server.end();
	lo_server.join();

	return 0;
}
