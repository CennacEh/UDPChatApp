compiler=g++
bothlibraries=-lws2_32
clientlibs=$(bothlibraries) -lraylib -lopengl32 -lgdi32 -lwinmm -lglfw3
flags=-Wall
src=./src
cliento = ./build/client.exe
servero = ./build/server.exe

all: compile

compile: client server

client:
	$(compiler) $(src)/client/client.cpp $(src)/client/gui.cpp -o $(cliento) $(clientlibs) $(flags)

server:
	$(compiler) $(src)/server/server.cpp -o $(servero) $(bothlibraries) $(flags)

clean:
	rm $(cliento) $(servero)