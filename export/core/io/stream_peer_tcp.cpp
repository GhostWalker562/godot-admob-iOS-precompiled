/*************************************************************************/
/*  stream_peer_tcp.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "stream_peer_tcp.h"

#include "core/project_settings.h"

Error StreamPeerTCP::_poll_connection() {

	ERR_FAIL_COND_V(status != STATUS_CONNECTING || !_sock.is_valid() || !_sock->is_open(), FAILED);

	Error err = _sock->connect_to_host(peer_host, peer_port);

	if (err == OK) {
		status = STATUS_CONNECTED;
		return OK;
	} else if (err == ERR_BUSY) {
		// Check for connect timeout
		if (OS::get_singleton()->get_ticks_msec() > timeout) {
			disconnect_from_host();
			status = STATUS_ERROR;
			return ERR_CONNECTION_ERROR;
		}
		// Still trying to connect
		return OK;
	}

	disconnect_from_host();
	status = STATUS_ERROR;
	return ERR_CONNECTION_ERROR;
}

void StreamPeerTCP::accept_socket(Ref<NetSocket> p_sock, IP_Address p_host, uint16_t p_port) {

	_sock = p_sock;
	_sock->set_blocking_enabled(false);

	timeout = OS::get_singleton()->get_ticks_msec() + (((uint64_t)GLOBAL_GET("network/limits/tcp/connect_timeout_seconds")) * 1000);
	status = STATUS_CONNECTING;

	peer_host = p_host;
	peer_port = p_port;
}

Error StreamPeerTCP::connect_to_host(const IP_Address &p_host, uint16_t p_port) {

	ERR_FAIL_COND_V(!_sock.is_valid(), ERR_UNAVAILABLE);
	ERR_FAIL_COND_V(_sock->is_open(), ERR_ALREADY_IN_USE);
	ERR_FAIL_COND_V(!p_host.is_valid(), ERR_INVALID_PARAMETER);

	Error err;
	IP::Type ip_type = p_host.is_ipv4() ? IP::TYPE_IPV4 : IP::TYPE_IPV6;

	err = _sock->open(NetSocket::TYPE_TCP, ip_type);
	ERR_FAIL_COND_V(err != OK, FAILED);

	_sock->set_blocking_enabled(false);

	timeout = OS::get_singleton()->get_ticks_msec() + (((uint64_t)GLOBAL_GET("network/limits/tcp/connect_timeout_seconds")) * 1000);
	err = _sock->connect_to_host(p_host, p_port);

	if (err == OK) {
		status = STATUS_CONNECTED;
	} else if (err == ERR_BUSY) {
		status = STATUS_CONNECTING;
	} else {
		ERR_PRINT("Connection to remote host failed!");
		disconnect_from_host();
		return FAILED;
	}

	peer_host = p_host;
	peer_port = p_port;

	return OK;
}

Error StreamPeerTCP::write(const uint8_t *p_data, int p_bytes, int &r_sent, bool p_block) {

	ERR_FAIL_COND_V(!_sock.is_valid(), ERR_UNAVAILABLE);

	if (status == STATUS_NONE || status == STATUS_ERROR) {

		return FAILED;
	}

	if (status != STATUS_CONNECTED) {

		if (_poll_connection() != OK) {

			return FAILED;
		}

		if (status != STATUS_CONNECTED) {
			r_sent = 0;
			return OK;
		}
	}

	if (!_sock->is_open())
		return FAILED;

	Error err;
	int data_to_send = p_bytes;
	const uint8_t *offset = p_data;
	int total_sent = 0;

	while (data_to_send) {

		int sent_amount = 0;
		err = _sock->send(offset, data_to_send, sent_amount);

		if (err != OK) {

			if (err != ERR_BUSY) {
				disconnect_from_host();
				return FAILED;
			}

			if (!p_block) {
				r_sent = total_sent;
				return OK;
			}

			// Block and wait for the socket to accept more data
			err = _sock->poll(NetSocket::POLL_TYPE_OUT, -1);
			if (err != OK) {
				disconnect_from_host();
				return FAILED;
			}
		} else {

			data_to_send -= sent_amount;
			offset += sent_amount;
			total_sent += sent_amount;
		}
	}

	r_sent = total_sent;

	return OK;
}

Error StreamPeerTCP::read(uint8_t *p_buffer, int p_bytes, int &r_received, bool p_block) {

	if (!is_connected_to_host()) {

		return FAILED;
	}

	if (status == STATUS_CONNECTING) {

		if (_poll_connection() != OK) {

			return FAILED;
		}

		if (status != STATUS_CONNECTED) {
			r_received = 0;
			return OK;
		}
	}

	Error err;
	int to_read = p_bytes;
	int total_read = 0;
	r_received = 0;

	while (to_read) {

		int read = 0;
		err = _sock->recv(p_buffer + total_read, to_read, read);

		if (err != OK) {

			if (err != ERR_BUSY) {
				disconnect_from_host();
				return FAILED;
			}

			if (!p_block) {
				r_received = total_read;
				return OK;
			}

			err = _sock->poll(NetSocket::POLL_TYPE_IN, -1);

			if (err != OK) {
				disconnect_from_host();
				return FAILED;
			}

		} else if (read == 0) {

			disconnect_from_host();
			r_received = total_read;
			return ERR_FILE_EOF;

		} else {

			to_read -= read;
			total_read += read;

			if (!p_block) {
				r_received = total_read;
				return OK;
			}
		}
	}

	r_received = total_read;

	return OK;
}

void StreamPeerTCP::set_no_delay(bool p_enabled) {

	ERR_FAIL_COND(!is_connected_to_host());
	_sock->set_tcp_no_delay_enabled(p_enabled);
}

bool StreamPeerTCP::is_connected_to_host() const {

	return _sock.is_valid() && _sock->is_open() && (status == STATUS_CONNECTED || status == STATUS_CONNECTING);
}

StreamPeerTCP::Status StreamPeerTCP::get_status() {

	if (status == STATUS_CONNECTING) {
		_poll_connection();
	} else if (status == STATUS_CONNECTED) {
		Error err;
		err = _sock->poll(NetSocket::POLL_TYPE_IN, 0);
		if (err == OK) {
			// FIN received
			if (_sock->get_available_bytes() == 0) {
				disconnect_from_host();
				return status;
			}
		}
		// Also poll write
		err = _sock->poll(NetSocket::POLL_TYPE_IN_OUT, 0);
		if (err != OK && err != ERR_BUSY) {
			// Got an error
			disconnect_from_host();
			status = STATUS_ERROR;
		}
	}

	return status;
}

void StreamPeerTCP::disconnect_from_host() {

	if (_sock.is_valid() && _sock->is_open())
		_sock->close();

	timeout = 0;
	status = STATUS_NONE;
	peer_host = IP_Address();
	peer_port = 0;
}

Error StreamPeerTCP::put_data(const uint8_t *p_data, int p_bytes) {

	int total;
	return write(p_data, p_bytes, total, true);
}

Error StreamPeerTCP::put_partial_data(const uint8_t *p_data, int p_bytes, int &r_sent) {

	return write(p_data, p_bytes, r_sent, false);
}

Error StreamPeerTCP::get_data(uint8_t *p_buffer, int p_bytes) {

	int total;
	return read(p_buffer, p_bytes, total, true);
}

Error StreamPeerTCP::get_partial_data(uint8_t *p_buffer, int p_bytes, int &r_received) {

	return read(p_buffer, p_bytes, r_received, false);
}

int StreamPeerTCP::get_available_bytes() const {

	ERR_FAIL_COND_V(!_sock.is_valid(), -1);
	return _sock->get_available_bytes();
}

IP_Address StreamPeerTCP::get_connected_host() const {

	return peer_host;
}

uint16_t StreamPeerTCP::get_connected_port() const {

	return peer_port;
}

Error StreamPeerTCP::_connect(const String &p_address, int p_port) {

	IP_Address ip;
	if (p_address.is_valid_ip_address()) {
		ip = p_address;
	} else {
		ip = IP::get_singleton()->resolve_hostname(p_address);
		if (!ip.is_valid())
			return ERR_CANT_RESOLVE;
	}

	return connect_to_host(ip, p_port);
}

void StreamPeerTCP::_bind_methods() {

	ClassDB::bind_method(D_METHOD("connect_to_host", "host", "port"), &StreamPeerTCP::_connect);
	ClassDB::bind_method(D_METHOD("is_connected_to_host"), &StreamPeerTCP::is_connected_to_host);
	ClassDB::bind_method(D_METHOD("get_status"), &StreamPeerTCP::get_status);
	ClassDB::bind_method(D_METHOD("get_connected_host"), &StreamPeerTCP::get_connected_host);
	ClassDB::bind_method(D_METHOD("get_connected_port"), &StreamPeerTCP::get_connected_port);
	ClassDB::bind_method(D_METHOD("disconnect_from_host"), &StreamPeerTCP::disconnect_from_host);
	ClassDB::bind_method(D_METHOD("set_no_delay", "enabled"), &StreamPeerTCP::set_no_delay);

	BIND_ENUM_CONSTANT(STATUS_NONE);
	BIND_ENUM_CONSTANT(STATUS_CONNECTING);
	BIND_ENUM_CONSTANT(STATUS_CONNECTED);
	BIND_ENUM_CONSTANT(STATUS_ERROR);
}

StreamPeerTCP::StreamPeerTCP() :
		_sock(Ref<NetSocket>(NetSocket::create())),
		timeout(0),
		status(STATUS_NONE),
		peer_port(0) {
}

StreamPeerTCP::~StreamPeerTCP() {

	disconnect_from_host();
}
