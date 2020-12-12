INC = -I. -IDataModels -IParser -IUtils -IWriter
SRC = *.cpp **/*.cpp

# .PHONY: parser

parser: $(SRC) *.h **/*.h
	g++ $(INC) $(SRC)