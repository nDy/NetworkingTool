/*
 * ProyectoRedesO.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: ndy
 */

#include <iostream>

#include <zmq.h>
#include <zmq_utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 5) {
		cout << "Uso incorrecto, Revisar la documentacion" << endl;
		return 1;
	}

	if (atoi(argv[4]) < 1 || atoi(argv[4]) > 6) {
		cout << "Uso incorrecto, Revisar la documentacion" << endl;
		return 1;
	}

	int roundtrip_count;
	size_t message_size;
	void *ctx;
	void *s;
	int rc;
	int i;
	zmq_msg_t msg;
//Tried switch but returns redefined variables on bind_to and similar
	if (atoi(argv[4]) == 1 || atoi(argv[4]) == 5) { //Local Ping & pkg loss
		if (atoi(argv[4]) == 1)
			cout << "!!!Local Ping!!!" << endl;
		else
			cout << "!!!Local Package Loss!!!" << endl;

		const char *bind_to;

		bind_to = argv[1];
		message_size = atoi(argv[2]);
		roundtrip_count = atoi(argv[3]);

		ctx = zmq_init(1);
		if (!ctx) {
			printf("error in zmq_init: %s\n", zmq_strerror(errno));
			return -1;
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

		rc = zmq_msg_init_size(&msg,message_size);
		if (rc != 0) {
			printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
			return -1;
		}

		for (i = 0; i != roundtrip_count; i++) {
			rc = zmq_recv(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("message of incorrect size received\n");
				return -1;
			}
			rc = zmq_send(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
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

		rc = zmq_term(ctx);
		if (rc != 0) {
			printf("error in zmq_term: %s\n", zmq_strerror(errno));
			return -1;
		}

	} else if (atoi(argv[4]) == 2) { //Remote Ping
		cout << "!!!Remote Ping!!!" << endl;

		const char *connect_to;
		void *watch;
		unsigned long elapsed;
		double latency;

		connect_to = argv[1];
		message_size = atoi(argv[2]);
		roundtrip_count = atoi(argv[3]);

		ctx = zmq_init(1);
		if (!ctx) {
			printf("error in zmq_init: %s\n", zmq_strerror(errno));
			return -1;
		}

		s = zmq_socket(ctx, ZMQ_REQ);
		if (!s) {
			printf("error in zmq_socket: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_connect(s, connect_to);
		if (rc != 0) {
			printf("error in zmq_connect: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_msg_init_size(&msg, message_size);
		if (rc != 0) {
			printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
			return -1;
		}
		memset(zmq_msg_data(&msg), 0, message_size);

		watch = zmq_stopwatch_start();

		for (i = 0; i != roundtrip_count; i++) {
			watch = zmq_stopwatch_start();
			rc = zmq_send(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			rc = zmq_recv(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("message of incorrect size received\n");
				return -1;
			}
			elapsed = zmq_stopwatch_stop(watch);
			printf("latency:%lu\n",elapsed);
			elapsed = 0;
		}

		elapsed = zmq_stopwatch_stop(watch);

		rc = zmq_msg_close(&msg);
		if (rc != 0) {
			printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		latency = (double) elapsed / (roundtrip_count * 2);

		printf("msgSize:%d\n", (int) message_size);
		printf("roundCount: %d\n", (int) roundtrip_count);
		printf("averageLatency: %.3f\n", (double) latency);

		rc = zmq_close(s);
		if (rc != 0) {
			printf("error in zmq_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_term(ctx);
		if (rc != 0) {
			printf("error in zmq_term: %s\n", zmq_strerror(errno));
			return -1;
		}

	} else if (atoi(argv[4]) == 3) { //Local Throughput
		cout << "!!!Local Throughput!!!" << endl;

		const char *bind_to;
		int message_count;
		void *watch;
		
		bind_to = argv[1];
		message_size = atoi(argv[2]);
		message_count = atoi(argv[3]);

		ctx = zmq_init(1);
		if (!ctx) {
			printf("error in zmq_init: %s\n", zmq_strerror(errno));
			return -1;
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

		rc = zmq_msg_init_size(&msg,message_size);
		if (rc != 0) {
			printf("error in zmq_msg_init: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_recv(s, &msg,message_size, 0);
		if (rc < 0) {
			printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
			return -1;
		}
		if (zmq_msg_size(&msg) != message_size) {
			printf("message of incorrect size received\n");
			return -1;
		}

		watch = zmq_stopwatch_start();

		for (i = 0; i != message_count - 1; i++) {
			rc = zmq_recv(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("message of incorrect size received\n");
				return -1;
			}
		}

		zmq_stopwatch_stop(watch);

		rc = zmq_msg_close(&msg);
		if (rc != 0) {
			printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_close(s);
		if (rc != 0) {
			printf("error in zmq_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_term(ctx);
		if (rc != 0) {
			printf("error in zmq_term: %s\n", zmq_strerror(errno));
			return -1;
		}
	} else if (atoi(argv[4]) == 4) { //Remote Throughput
		cout << "!!!Remote Throughput!!!" << endl;

		const char *connect_to;

		int message_count;
		void *watch;
		unsigned long elapsed;
		unsigned long throughput;
		double megabits;

		connect_to = argv[1];
		message_size = atoi(argv[2]);
		message_count = atoi(argv[3]);

		ctx = zmq_init(1);
		if (!ctx) {
			printf("error in zmq_init: %s\n", zmq_strerror(errno));
			return -1;
		}

		s = zmq_socket(ctx, ZMQ_REQ);
		if (!s) {
			printf("error in zmq_socket: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_connect(s, connect_to);
		if (rc != 0) {
			printf("error in zmq_connect: %s\n", zmq_strerror(errno));
			return -1;
		}

//watch
		watch = zmq_stopwatch_start();
		for (i = 0; i != message_count; i++) {
			rc = zmq_msg_init_size(&msg, message_size);
			if (rc != 0) {
				printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
				return -1;
			}
			rc = zmq_send(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			rc = zmq_msg_close(&msg);
			if (rc != 0) {
				printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
				return -1;
			}
		}

		elapsed = zmq_stopwatch_stop(watch);
		if (elapsed == 0)
			elapsed = 1;
		
		throughput = (unsigned long) ((double) message_count / (double) elapsed
				* 1000000);
		megabits = (double) (throughput * message_size * 8) / 1000000;

		printf("message size: %d [B]\n", (int) message_size);
		printf("message count: %d\n", (int) message_count);
		printf("mean throughput: %d [msg/s]\n", (int) throughput);
		printf("mean throughput: %.3f [Mb/s]\n", (double) megabits);

		rc = zmq_close(s);
		if (rc != 0) {
			printf("error in zmq_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_term(ctx);
		if (rc != 0) {
			printf("error in zmq_term: %s\n", zmq_strerror(errno));
			return -1;
		}

	} else if (atoi(argv[4]) == 6) { //Remote Ping
		cout << "!!!Remote Package Loss!!!" << endl;

		const char *connect_to;
		void *watch;
		unsigned long elapsed;
		double latency;

		connect_to = argv[1];
		message_size = atoi(argv[2]);
		roundtrip_count = atoi(argv[3]);

		ctx = zmq_init(1);
		if (!ctx) {
			printf("error in zmq_init: %s\n", zmq_strerror(errno));
			return -1;
		}

		s = zmq_socket(ctx, ZMQ_REQ);
		if (!s) {
			printf("error in zmq_socket: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_connect(s, connect_to);
		if (rc != 0) {
			printf("error in zmq_connect: %s\n", zmq_strerror(errno));
			return -1;
		}
		rc = zmq_msg_init_size(&msg, message_size);
		if (rc != 0) {
			printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
			return -1;
		}
		memset(zmq_msg_data(&msg), 0, message_size);

		watch = zmq_stopwatch_start();

		for (i = 0; i != roundtrip_count; i++) {
			watch = zmq_stopwatch_start();
			rc = zmq_send(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			rc = zmq_recv(s, &msg,message_size, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("message of incorrect size received\n");
				return -1;
			}
			elapsed = zmq_stopwatch_stop(watch);
			printf("latency:%lu\n",elapsed);
			elapsed = 0;
		}

		

		rc = zmq_msg_close(&msg);
		if (rc != 0) {
			printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
			return -1;
		}


		rc = zmq_close(s);
		if (rc != 0) {
			printf("error in zmq_close: %s\n", zmq_strerror(errno));
			return -1;
		}

		rc = zmq_term(ctx);
		if (rc != 0) {
			printf("error in zmq_term: %s\n", zmq_strerror(errno));
			return -1;
		}
	}
	return 0;
}
