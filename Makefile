all: client server

client:
	g++ -o build/client src/client.cpp src/util.cpp

server:
	g++ -o build/server src/server.cpp src/util.cpp

clean:
	rm -rf build/*
