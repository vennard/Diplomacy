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
  	//just handling one save game -- case statement for more
	printf("Starting new game...");
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
        int or = o[i].order;

	  	if (g[c].player != p) continue; //Player doesn't match 
        if (g[c].occupy_type == -1) continue; //Region doesn't have unit
        switch (or) {
            case 0 : //hold
	  		    printf("Order #%i which is a hold is valid!\r\n",i);
                validOrders++;
                o[i].valid = 1;
                g[c].dS++;
                continue;
                break;
            case 1 : //move
                if ((g[c].occupy_type == 0)&&(g[tc].type == 2)) continue;
                if ((g[c].occupy_type == 1)&&(g[tc].type == 0)) continue;
                if (!isneighbor(tc,g[c].ncountrys)) continue;
	  		        printf("Order #%i which is a troop issued move is valid!\r\n",i);
                    validOrders++;
                    o[i].valid = 1;
                    g[tc].aS++; //++attack strength to target country
                break;
            case 2 : //support
                //land units cant support water region
                if ((g[c].occupy_type == 0)&&(g[sc].type == 2)) continue; 
                //fleet cant support inland region
                if ((g[c].occupy_type == 1)&&(g[sc].type == 0)) continue;
                if (g[sc].occupy_type == -1) continue; 
                if (!isneighbor(sc,g[c].ncountrys)) continue; 
                if (tc == -1) { //supporting a hold,convoy, or support
	  		        printf("Order #%i which is a support of a hold, support, or convoy is valid!\r\n",i);
                    o[i].valid = 1;
                    g[sc].dS++; //++defense strength of support country
                    validOrders++;
                    continue;
                } else { //supporting a move
                    //sc and tc must be neighbors of eachother and c
                    if (!isneighbor(tc,g[sc].ncountrys)) continue;
                    if (!isneighbor(tc,g[c].ncountrys)) continue;
	  		        printf("Order #%i which is a support of a move is valid!\r\n",i);
                    o[i].valid = 1;
                    g[tc].aS++; //++attack strength to target country
                    validOrders++;
                    continue;
                }
                break;
            case 3 : //convoy
                if (g[c].occupy_type == 0) { //land units convoy order
                    if (g[c].type != 1) continue; //region must be coastal
                    //land unit must be neighbor of fleet & a fleet must exist
                    if ((!isneighbor(c, g[sc].ncountrys))||(g[sc].occupy_type != 1)) continue; 
                    //TODO complicated target country check
                    //Must recursively check through countrys neighboring sc
                    //checking for fleets then check through those neighbors etc
                    //until the tc is found or we run out of fleets to search
                    //if all those pass then this convoy order is valid
	  		        printf("Order #%i which is a troop issued convoy is valid!\r\n",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                } else if (g[c].occupy_type == 1) { //fleets convoy order
                    if (g[c].type != 2) continue; //region must be water
                    if (g[sc].type != 1) continue; //troop must be coastal
                    if (g[sc].occupy_type != 0) continue; //troop must exist
                    //use same recursive search as above TODO
	  		        printf("Order #%i which is a fleet issued convoy is valid!\r\n",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                } else {
                    continue;
                }
                break;
            default :
                break;
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
}
