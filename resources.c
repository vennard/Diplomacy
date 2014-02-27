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
#include "include.h"

region_t g[48];
order_t o[MAX_ORDERS];
int validOrders = 0;
int numO = 0;

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

int isneighbor(int c, int *nc) {
    int k = 0;
    int valid = 0;
	while (nc[k] != -1) {
	   if (nc[k] == c) valid = 1;  	
		k++;
	}
    return valid;
}

//First Run through validation check
int firstvalidate(void) {
	//Check if order is valid
	int i;
	printf("Start of 1st Round validation with %i orders!...\r\n",numO);
	for (i = 0;i < numO;i++) {
	  	int p = o[i].player;
	  	int c = o[i].country;
		int t = o[i].type;
		int tc = o[i].tcountry;
		int sc = o[i].scountry;
	  	if (g[c].player != p) continue; //Player doesn't match 
        if (g[c].occupy_type == -1) continue; //Region doesn't have unit

        int or = o[i].order;
        switch (or) {
            case 0 : //hold
                break;
            case 1 : //move
                break;
            case 2 : //support
                break;
            case 3 : //convoy
                if (g[c].occupy_type == 0) { //land units convoy order
                    if (g[c].type != 1) continue;
                } else if (g[c].occupy_type == 1) { //fleets convoy order
                    if (g[c].type != 2) continue;
                } else {
                    continue;
                }

                break;
            default :
                break;
        }

    }

        /*
	  		//printf("Order #%i: Player does not have unit at orders country! \r\n",i);
			continue;
		} 
		//Check that region type is correct
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
        if (!isneighbor(tc, g[c].ncountrys)) continue;
       
		//Order is a convoy
		if (o[i].order == 3) {
			//check that scountry has a unit
			if (g[sc].occupy_type == -1) continue;	
			//check that scountry has correct unit type
			if (g[sc].type == 2) continue;  
			//check that tcountry is correct type 
			if (g[tc].type == 2) continue;
			//check that scountry (location of land unit to be moved) is valid
			int validconvoy = 0;
			int a = 0;
			while (nc[a] != -1) {
				if (nc[a] == sc) validconvoy = 1;	
			  	a++;
			}
			if (!validconvoy) continue; 
			printf("Order #%i which is a convoy is valid!\r\n",i);
			validOrders++;
			o[i].valid = 1;
			continue;
		}
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

	}
	printf("Number of valid orders after 1st validation = %i\r\n",validOrders);
	int j;
	for(j = 0;j < numO;j++) {
	  if (o[j].valid == 1) {
		printf("Order #%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",j,o[j].player,o[j].order,o[j].type,o[j].country);
		printf("To Country - %i, Support Country - %i.\r\n",o[j].tcountry,o[j].scountry);
	  }
	}
	return validOrders;
    */
    return 0;
}
