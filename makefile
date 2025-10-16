compiler=g++
sharedlibraries=-lws2_32
clientlibs=$(sharedlibraries) -lraylib -lopengl32 -lgdi32 -lwinmm -lglfw3
flags=-Wall
src=./src
cliento = ./build/client/client.exe
servero = ./build/server/server.exe
clientsources := $(shell find $(src)/client -name "*.cpp")
clientheaders := $(shell find $(src)/client -name "*.h")

all: compile

compile: client server

client: $(clientsources) $(clientheaders)
	@echo "Compiling client..."
	$(compiler) $(clientsources) -o $(cliento) $(clientlibs) $(flags)
	@echo "Client compiling done!"

server: $(src)/server/server.cpp $(src)/server/logging/log.cpp
	@echo "Compiling server..."
	$(compiler) $(src)/server/server.cpp $(src)/server/logging/log.cpp -o $(servero) $(sharedlibraries) $(flags)
	@echo "Server compiling done!"

clean:
	rm $(cliento) $(servero)
	@echo "Output files deleted!"