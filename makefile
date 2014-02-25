HEADERS = order.h region.h
OBJECTS = arbitrator.c testorders.c startgame.c

default: arbitrator

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

arbitrator: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

clean:
	-rm -f $(OBJECTS)
