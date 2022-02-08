CC = gcc -c
LD = gcc
CFLAGS = -O2
LIBS = -lpaho-mqtt3c

receiver: telemetry.o xbee_tools.o
	$(LD) -o $@ $^ $(LIBS)

telemetry.o: telemetry.c
	$(CC) $(CFLAGS) $< -o $@

xbee_tools.o: ../Libraries/xbee_tools/xbee_tools.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -vf *.o receiver
