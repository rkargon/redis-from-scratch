all: client server

client:
	g++ -Wall -Wextra --std=c++1z -O2 -o build/client src/client.cpp src/util.cpp

server:
	g++ -Wall -Wextra --std=c++1z -O2 -o build/server src/server.cpp src/util.cpp

clean:
	rm -rf build/*
