all:
	g++ client.cc ../../stream_socket.cc ../socket_object.cc ../object_base.cc -g ../../../thread_que.cc -L../../../libmhthread/ -lmhthread -lpthread -o demo
	gcc send_server.c -o snd
	gcc recv_server.c -o rcv
