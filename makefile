HEADERS = order.h region.h include.h
OBJECTS = arbitrator.c testorders.c startgame.c resources.c

default: arbitrator 

arbitrator: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

testorders: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

startgame: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

clean:
	-rm -f arbitrator testorders startgame *.o 
