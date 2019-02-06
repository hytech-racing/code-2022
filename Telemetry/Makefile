CC = gcc -c
LD = gcc
CFLAGS = -O2
LIBS = -lpaho-mqtt3c

receiver: telemetry.o XBTools.o
	$(LD) $^ $(LIBS)

telemetry.o: telemetry.c
	$(CC) $(CFLAGS) $< -o $@

XBTools.o: ../Libraries/XBTools/XBTools.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -vf *.o receiver
