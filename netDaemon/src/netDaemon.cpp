#include <iostream>
#include <zmq.h>
#include <zmq_utils.h>
int main(int argc, char **argv) {
	//zmq parameters init

	int roundtrip_count;
	size_t message_size;
	void *ctx;
	void *s;
	int rc;
	zmq_msg_t msg;
	const char *bind_to;
	bind_to = argv[1];
	message_size = 29;
	roundtrip_count = 100;

	//zmq init
	ctx = zmq_init(1);
	if (!ctx) {
		std::cout << "error in zmq_init: " << zmq_strerror(errno) << std::endl;
	} else {
		std::cout << "zmq start successful" << std::endl;
	}

	s = zmq_socket(ctx, ZMQ_REP);
	if (!s) {
		printf("error in zmq_socket: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_bind(s, bind_to);
	if (rc != 0) {
		printf("error in zmq_bind: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_msg_init_size(&msg, message_size);
	if (rc != 0) {
		printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
		return -1;
	}

	while (true) {

		for (int i = 0; i != roundtrip_count; i++) {
			rc = zmq_recv(s, &msg, message_size, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("message of incorrect size received\n");
				return -1;
			}
			rc = zmq_send(s, &msg, message_size, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
		}
	}

	rc = zmq_msg_close(&msg);
	if (rc != 0) {
		printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
		return -1;
	}

	zmq_sleep(1);

	rc = zmq_close(s);
	if (rc != 0) {
		printf("error in zmq_close: %s\n", zmq_strerror(errno));
		return -1;
	}

	//zmq closure
	rc = zmq_term(ctx);
	if (rc != 0) {
		std::cout << "error in zmq_term:" << zmq_strerror(errno) << std::endl;
	} else {
		std::cout << "zmq exit successful" << std::endl;
	}
	return 0;
}
