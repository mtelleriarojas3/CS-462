all: server client

server: includes.h includes.cpp server.cpp
	g++ -std=c++11 -pthread includes.cpp server.cpp -o server

client: includes.h includes.cpp client.cpp
	g++ -std=c++11 -pthread includes.cpp client.cpp -o client

clean: server client
	rm -f server client