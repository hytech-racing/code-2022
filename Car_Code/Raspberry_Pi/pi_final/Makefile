CXX = g++
FLAGS = -w -std=c++11

all: main_can
logger: can_logger

main_can: hytech17.o can_lib.o bt_lib.o main_can.o
	$(CXX) $(FLAGS) -o main_can BMS_temperatures.o PCU_status.o PCU_voltages.o TCU_status.o TCU_readings.o main_can.o can_lib.o bt_lib.o

can_logger: hytech17.o can_lib.o can_logger.o
	$(CXX) $(FLAGS) -o can_logger MC_torque_timer_information.o can_logger.o can_lib.o


# Object files

hytech17.o: ../../../Libraries/HyTech17_Library/*.cpp
	$(CXX) $(FLAGS) -c ../../../Libraries/HyTech17_Library/MC_torque_timer_information.cpp

can_lib.o: can_lib.cpp
	$(CXX) $(FLAGS) -c can_lib.cpp

bt_lib.o: bt_lib.cpp
	$(CXX) $(FLAGS) -c bt_lib.cpp

main_can.o: main_can.cpp
	$(CXX) $(FLAGS) -c main_can.cpp

can_logger.o: can_logger.cpp
	$(CXX) $(FLAGS) -c can_logger.cpp

clean:
	-rm -f ../../HyTech17_Library/*.o
	-rm -f *.o
	-rm -f main_can
	-rm -f can_logger

remake: clean all

clear_logs:
	-rm -f logs_*
