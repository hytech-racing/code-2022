CXX = g++
FLAGS = -w -std=c++11

all: xbee_rcv

xbee_rcv: HyTech17.o XBTools.o
	$(CXX) $(FLAGS) -o xbee_rcv xbee_rcv.cpp XBTools.o

HyTech17.o: ../../../Libraries/HyTech17_Library/*.cpp
	$(CXX) $(FLAGS) -c ../../../Libraries/HyTech17_Library/*.cpp

XBTools.o: ../../../Libraries/XBTools/XBTools.cpp
	$(CXX) $(FLAGS) -c ../../../Libraries/XBTools/XBTools.cpp

clean: 
	rm -f ../../../Libraries/HyTech17_Library/*.o
	rm -f ../../../Libraries/XBTools/*.o
	rm -f *.o
	rm -f xbee_rcv

remake: clean all
