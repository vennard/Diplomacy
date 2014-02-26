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

#define MAX_ORDERS 20000
//Creating sample output from the controllers
// Sending string (bytes)
// Player Num (1) / Order (1) / Unit Location (3) / [To Country (3)]
//
// Player Num = 1 -> 4
// Orders: 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy // BY DEFAULT 000 = NULL (represents no country)


region_t g[48];
order_t o[MAX_ORDERS];
int loadgamedata(char f[]) {
   int fd = open(f, O_RDONLY);
   if (fd < 0) {
  		perror("read");
  		exit(1);
   }
	if (read(fd,g,sizeof(g)) < 0) perror("read");
	printf("Read success!\r\n");
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
  
//returns number of orders
int getTestOrders(int seed, int numOrders, char f[]) {
	genOrders(seed,numOrders,f);
   int fd1 = open(f, O_RDONLY);
   if (fd1 < 0) {
  		perror("read");
		return -1;
   }
	//get size of file
	struct stat st;
	fstat(fd1,&st);
	int size = st.st_size;
	numOrders = size / sizeof(order_t);
	if (read(fd1, o, size) < 0) {
	  printf("Reading orders failed!\r\n");
	  return -1;
	}
	return numOrders;
}

char fgdata[] = {"/tmp/gamedata"};
char forders[] = {"/tmp/outfile"};
int main(int argc, char *argv[]) {
   if (argc > 3) perror("too many args");
	printf("Starting the Arbitrator...\r\n");
	startnewgame(0);
	int numO = getTestOrders(5,5000,forders);
	int validOrders = 0;
	//Check if order is valid
	int i;
	printf("Start of Testing with %i orders!...\r\n",numO);
	for (i = 0;i < numO;i++) {
	  	//Check that player has unit at order location (country)
	  	int p = o[i].player;
	  	int c = o[i].country;
	  	if (g[c].player != p) {
	  		//printf("Order #%i: Player does not have unit at orders country! \r\n",i);
			continue;
		} 
		//Check that region type is correct
		int t = o[i].type;
		if (t != g[c].occupy_type) {
	  		//printf("Order #%i: Troop type does not match! \r\n",i);
			continue;
		}
		//Order is a hold
		if (o[i].order == 0) {
			o[i].valid = 1;
	  		printf("Order #%i which is a hold is valid! \r\n",i);
			validOrders++;
			continue;
		}
		//check that target country is a neighbor of troop country
		int tc = o[i].tcountry;
		int k = 0;
		int validtcountry = 0;
		int *nc; 
	  	nc = g[c].ncountrys;
		while (nc[k] != -1) {
		   if (nc[k] == tc) validtcountry = 1;  	
			k++;
		}
		if (validtcountry != 1) continue;
		//check that target country is correct type
		int validtype = 0;
		if (t == 0) {   	//if land unit
		  	if ((g[tc].type == 0)||(g[tc].type == 1)) {
				validtype = 1;	 	
		  	}
		} else if (t == 1) { //if fleet unit
		  	if ((g[tc].type == 1)||(g[tc].type == 2)) {
				validtype = 1;	 	
		  	}
		} else {
			perror("incorrect order type");
	   }
		if (validtype != 1) continue;
		
		//Order is a move
		if (o[i].order == 1) {
	  		printf("Order #%i which move is valid! \r\n",i);
			validOrders++;
			o[i].valid = 1;
			continue;
		}
		//Order is a support
		if (o[i].order == 2) {
		  //check that target country is neighbor of support country
		  int sc = o[i].scountry;
		  int f = 0;
		  int validsupport = 0;
		  nc = g[tc].ncountrys;
		  while (nc[f] != -1) {
			 if (nc[f] == sc) {
				validsupport = 1;
			 }
			 f++;
		  }
		  if (validsupport == 1) {
				printf("Order #%i which is a support is valid!\r\n",i);
				o[i].valid = 1;
				validOrders++;
				continue;
		  }
		}
		//Order is a convoy
		if (o[i].order == 3) {
		}

	}
	printf("Number of valid orders after 1st validation = %i\r\n",validOrders);
	int j;
	for(j = 0;j < numO;j++) {
	  if (o[j].valid == 1) {
		printf("Order #%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",j,o[j].player,o[j].order,o[j].type,o[j].country);
		printf("To Country - %i, Support Country - %i.\r\n",o[j].tcountry,o[j].scountry);
	  }
	}
   return 0;
}
