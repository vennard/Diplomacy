OBJECTS= $(patsubst %.c,%.o, $(wildcard *.c))
#CFLAGS= -Wall -Wextra -Werror

diplomacy: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o diplomacy

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d  $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	 sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp


clean:
	-rm -f diplomacy testorders startgame *.o 
