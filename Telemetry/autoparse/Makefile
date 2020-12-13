INC = -Iinclude -Iinclude/DataModels
SRC = src/*.cpp src/Parser/*.cpp

a.out: $(SRC) include/*.h include/DataModels/*.h 
	rm -f a.out
	g++ $(INC) $(SRC)

.PHONY: parser
parser: a.out
	rm -rf build
	mkdir build
	cp -r templates/* build
	./a.out

.PHONY: clean
clean:
	rm -rf build