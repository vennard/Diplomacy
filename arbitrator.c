#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "order.h"
#include "region.h"

//Creating sample output from the controllers
// Sending string (bytes)
// Player Num (1) / Order (1) / Unit Location (3) / [To Country (3)]
//
// Player Num = 1 -> 4
// Orders: 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
// BY DEFAULT 000 = NULL (represents no country)


region_t g[48];
char fgdata[] = {"/tmp/gamedata"};
char forders[] = {"/tmp/outfile"};
int loadgamedata(char f[]) {
   int fd = open(f, O_RDONLY);
   if (fd < 0) {
  		perror("read");
  		exit(1);
   }
	if (read(fd,g,sizeof(g)) < 0) perror("read");
	printf("Read success, first 35 occupy type= %d\r\n",g[35].occupy_type);
	return 0;
}

int startnewgame(int num) {
	printf("Starting new game...");
  	//just handling one save game -- case statement for more
	char fname[] = "/tmp/savegame";
	if (num == 0) {
	  //fname = "/tmp/savegame1";
	} else {
		return 1;	
	}
	printf(" saved as: %s\r\n",fname);
	genStart(fname); //MUST BE CHANGED TO PERMANENT LOCATION
	loadgamedata(fname);
	return 0;
}
  

int main(int argc, char *argv[]) {
	genOrders(2,2,forders);
	genStart(fgdata);
	printf("Start of Testing...\r\n");
	startnewgame(0);

	

	/*
	// open game orders file
   int fd1 = open(forders, O_RDONLY);
   if (fd1 < 0) {
  		perror("read");
  		exit(1);
   }
	*/



   return 0;
}
