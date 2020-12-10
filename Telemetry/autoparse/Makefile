INC = -I. -IDataModels -IParser -IUtils
SRC = *.cpp **/*.cpp

.PHONY: parser

parser: $(SRC) *.h **/*.h
	g++ $(INC) $(SRC)