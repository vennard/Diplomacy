HEADERS = order.h region.h include.h
OBJECTS = arbitrator.c testorders.c startgame.c resources.c diplomacy.c rxorders.c
SAFE= -Wall -Werror

default: diplomacy

arbitrator: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

testorders: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

startgame: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

resources: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

diplomacy: $(OBJECTS)
	gcc $(OBJECTS) -o $@ 

clean:
	-rm -f diplomacy testorders startgame *.o 
