HEADERS = order.h region.h include.h
OBJECTS = arbitrator.c testorders.c startgame.c resources.c diplomacy.c rxorders.c

default: diplomacy

arbitrator: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

testorders: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

startgame: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

resources: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

diplomacy: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

clean:
	-rm -f diplomacy testorders startgame *.o 
