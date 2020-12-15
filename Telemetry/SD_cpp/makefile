sd_reader:
	g++ --std=c++11 sd_reader.cpp -o sd_reader

auto_sd:
	g++ -I../../Libraries/HyTech_CAN -I../autoparse/build -I../autoparse/include sd_reader_autoparse.cpp ../autoparse/build/*.cpp ../autoparse/user/*.cpp