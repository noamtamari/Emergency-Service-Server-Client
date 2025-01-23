CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread
# -c: Compile only, do not link. -Wall: Enable all compiler warnings.-Weffc++: Enforce effective C++ programming practices.
# -g: Generate debug information. -std=c++11: Compile using the C++11 standard. -Iinclude: Add the include directory to the compiler's search path.
all: StompWCIClient

# EchoClient: bin/ConnectionHandler.o bin/echoClient.o
# 	g++ -o bin/EchoClient bin/ConnectionHandler.o bin/echoClient.o $(LDFLAGS)

# This rule links the object files into the final executable bin/StompWCIClient.
StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/keyboardInput.o bin/Frame.o bin/StompProtocol.o
	g++ -o bin/StompWCIClient bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/keyboardInput.o bin/Frame.o bin/StompProtocol.o $(LDFLAGS)

# This rule compiles the source file src/ConnectionHandler.cpp into the object file bin/ConnectionHandler.o.
bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

# bin/echoClient.o: src/echoClient.cpp
# 	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

bin/Event.o: src/Event.cpp
	g++ $(CFLAGS) -o bin/Event.o src/Event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/Frame.o: src/Frame.cpp
	g++ $(CFLAGS) -o bin/Frame.o src/Frame.cpp	

bin/keyboardInput.o: src/keyboardInput.cpp
	g++ $(CFLAGS) -o bin/keyboardInput.o src/keyboardInput.cpp	

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp	

# .PHONY indicates that clean is not a real file but a pseudo-target.
.PHONY: clean
clean:
	rm -f bin/*
	