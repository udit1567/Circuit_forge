#pragma once

#include "SocketWrapper.h"
#include "ZephyrClient.h"
#include "api/Server.h"
#include "unistd.h"

class ZephyrServer : public arduino::Server, ZephyrSocketWrapper {
private:
	int _port;

public:
	ZephyrServer() : _port(80) {};
	ZephyrServer(uint16_t port) : _port(port) {};

	virtual ~ZephyrServer() {
		end();
	}

	void end() {
		ZephyrSocketWrapper::close();
	}

	void begin(uint16_t port) {
		_port = port;
		begin();
	}

	void begin() {
		ZephyrSocketWrapper::bind(_port);
		ZephyrSocketWrapper::listen(5);
	}

	uint8_t status() {
		return 0;
	}

	explicit operator bool() {
		return sock_fd != -1;
	}

	ZephyrClient accept(uint8_t *status = nullptr) {
		ZephyrClient client;
		int sock = ZephyrSocketWrapper::accept();
		client.setSocket(sock);
		return client;
	}

	ZephyrClient available(uint8_t *status = nullptr) __attribute__((deprecated("Use accept()."))) {
		return accept(status);
	}

	size_t write(uint8_t c) override {
		return write(&c, 1);
	}

	size_t write(const uint8_t *buffer, size_t size) override {
		return send(buffer, size);
	}

	friend class ZephyrClient;
};
