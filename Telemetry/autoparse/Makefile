CC = g++ -std=c++14 -D_GNU_SOURCE
GCC = g++ -std=c++14 -D_GNU_SOURCE

INC_DIR = include
USR_DIR = user
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
TEMPLATES = templates/*
AP_OUT_DIR = $(BUILD_DIR)/auto

DATAMODELS = ClassDef FlagDef VarDef
__AP_HDR = AnnotationParser Canonicalize Exception InputBuffer StringUtils Token Writer $(addprefix DataModels/,$(DATAMODELS))
__AP_SRC = Canonicalize InputBuffer main Writer AP_classdef AP_flagdef AP_main AP_utils AP_vardef

AP_BIN = $(BUILD_DIR)/AnnotationParser
AP_INC = -I$(INC_DIR) -I$(INC_DIR)/DataModels
AP_HDR = $(patsubst %,$(INC_DIR)/%.h,$(__AP_HDR))
AP_OBJ = $(patsubst %,$(OBJ_DIR)/%.o,$(__AP_SRC))

HYTECH_CAN_LIB = ../../Libraries/HyTech_CAN

CAN_BIN = $(BUILD_DIR)/CANParser
CAN_INC = -I$(INC_DIR) -I$(AP_OUT_DIR) -I$(USR_DIR) -I$(HYTECH_CAN_LIB)
CAN_SRC = $(AP_OUT_DIR)/AutoParse.cpp $(AP_OUT_DIR)/ParseMessage.cpp $(USR_DIR)/*.cpp app/parser.cpp

install:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

build/obj/AP_%.o: src/AnnotationParser/AP_%.cpp
	$(CC) $(AP_INC) -c $< -o $@

build/obj/%.o: src/%.cpp
	$(CC) $(AP_INC) -c $< -o $@

$(AP_BIN): $(AP_OBJ) $(AP_HDR)
	$(CC) $(AP_OBJ) -o $(AP_BIN)

AnnotationParser: $(AP_BIN)

$(AP_OUT_DIR): $(AP_BIN) $(HYTECH_CAN_LIB)/*.h $(TEMPLATES)
	rm -rf $(AP_OUT_DIR)
	mkdir $(AP_OUT_DIR)
	cp -r $(TEMPLATES) $(AP_OUT_DIR)
	$(AP_BIN)

Compile: $(AP_OUT_DIR)

$(CAN_BIN): $(AP_OUT_DIR) $(USR_DIR) $(CAN_SRC) $(INC_DIR)
	$(CC) $(CAN_INC) $(CAN_SRC) -o $(CAN_BIN)

CANParser: $(CAN_BIN)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
