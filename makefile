compiler=g++
libraries=-lws2_32
flags=-static
src=./src/
cliento = ./build/client.exe
servero = ./build/server.exe

all: compile

compile: client server

client:
	$(compiler) $(src)client.cpp -o $(cliento) $(libraries) $(flags)

server:
	$(compiler) $(src)server.cpp -o $(servero) $(libraries) $(flags)

clean:
	rm $(cliento) $(servero)