CXX = g++
FLAGS = -w -std=c++11

all: xbee_msg_generator

xbee_msg_generator: HyTech_CAN.o XBTools.o 
	$(CXX) $(FLAGS) -o xbee_msg_generator xbee_msg_generator.cpp FCU_status.o XBTools.o

HyTech_CAN.o: ../../Libraries/HyTech_CAN/*.cpp
	$(CXX) $(FLAGS) -c ../../Libraries/HyTech_CAN/*.cpp

XBTools.o: ../../Libraries/XBTools/XBTools.cpp
	$(CXX) $(FLAGS) -c ../../Libraries/XBTools/XBTools.cpp

clean: 
	rm -f *.o
	rm -f xbee_rcv

remake: clean all
