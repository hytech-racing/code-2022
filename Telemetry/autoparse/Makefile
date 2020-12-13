INC = -Iinclude -Iinclude/DataModels
SRC = src/*.cpp src/Parser/*.cpp

.PHONY: parser

parser: $(SRC) include/*.h include/DataModels/*.h
	rm -rf build
	mkdir build
	cp -r templates/* build
	g++ $(INC) $(SRC)
	./a.out