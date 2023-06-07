server: server.c create_listener.c client_handler.c worker_handler.c
	gcc -o server server.c create_listener.c client_handler.c worker_handler.c -fsanitize=address -static-libasan -g