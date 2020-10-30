CC = g++ --std=c++11 -g -Wall
BOARD = -x c++ $(board)

.PHONY: depend
depend:
	bash depend

.PHONY: clean
clean:
	rm -rf Linker

include Libraries/.depend

Linker/main.o: main.cpp
	install -D /dev/null $@
	rm $@
	$(CC) -c $(LIBS) -o $@ $<

Linker/sim/%.o: Libraries/%.cpp
	install -D /dev/null $@
	rm $@
	$(CC) -c $(LIBS) -o $@ $<

Linker/ext/%.o: ../Libraries/%.cpp
	install -D /dev/null $@
	rm $@
	$(CC) -c $(LIBS) -o $@ $<

.PHONY: arduino
arduino:
ifeq ($(arduino),)
	$(error Specify arduino version | usage: make board=<boardfile> arduino=<version>)
else
	echo \#include \"BoardDefs/$(arduino).h\" > Libraries/Arduino/BoardDef.h
endif

.PHONY: simulate
simulate: arduino Linker/main.o $(DEPS)
ifeq ($(board),)
	$(error Specify board file | usage: make board=<boardfile> arduino=<version>)
else
	$(CC) $(LIBS) Linker/main.o $(DEPS) $(BOARD) -o $@
endif