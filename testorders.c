#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "region.h"
#include "order.h" //see file for definitions
#include "include.h"
//Creating sample output from the controllers
// Sending string (bytes)
// Player Num (1) / Order (1) / Unit Location (3) / [To Country (3)]
//
// Player Num = 1 -> 4
// Orders: 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
// BY DEFAULT -1 represents no country (invalid)
char *outFile   = "/tmp/outfile";
int ordersLeft = 0;
int* ptr;
int num[64];
int *nptr = num;

//search gameboard for players units of type
//returns number of units found
//0 - land, 1 - fleet
int findunits(int player, int type) {
    int c = 0;
    int i;
    for (i = 0;i < 48;i++) {
        if ((g[i].player == player)&&(g[i].occupy_type == type)) {
            num[c] = i;
            c++;
        }
    }
    return c;
}
//assumes gameboard is populated
int iorder(int s, int num, char out[]) {
    int a = 0;
    order_t r;
    ptr = (int *) malloc(sizeof(r));
	printf("Launching iOrder -- your very own \"ahem\" intelligent ordering system. Processing %i Orders..\r\n",num);
    if((num < 1)||(s < 0)||(strlen(out)<1)) return -1;
    srand(s);
    int k;
    for (k = 0;k < num;k++) {
        int *nc;
        int a = 0;
        int i = 0;
        r.player = rand() % 4;
        r.order = rand() % 4;
        r.type = rand() % 2;
        int numunits = findunits(r.player, r.type);
        int x = rand() % numunits;
        int country = nptr[x];
        r.country = country;
        switch (r.order) {
            case 0 : //hold
                break;
            case 1 : //move
                nc = g[country].ncountrys;
                while(a != -1) {
                    a = nc[i];
                    i++;
                }
                int m = rand() % i;
                r.tcountry = nc[m];
                break; 
            case 2 : //support
                break;
            case 3 : //convoy
                break;
        }
    }

    return 0;
}

int genOrders(int seed, int numOrders, char out[]) {
	int sd = 0;
	order_t r;

    ptr = (int *) malloc(sizeof(r));
	printf("Generating %i Orders..\r\n",numOrders);

	if (strlen(out) < 1) {
		printf("No outfile set, resorting to default /tmp/outfile\r\n");
	} else {
	  outFile = out;
   }

	if (seed < 0) {
		printf("Seed set was negative! Setting to 1.. \r\n");
	} else {
	  sd = seed; 
	}

	if (numOrders < 1) {
		printf("No orders set... thats not very adventurous - exiting\r\n");
		return 0;
	} else {
	  ordersLeft = numOrders;
   }

   // seed random number generator
   srand(sd);

   // open and create output file
   int fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
   if (fd < 0) {
  		perror("open");
  		return 1;
   }

   int i;
   for (i = 0; i < ordersLeft; i++) {
      r.player = rand() % 4;
      int order_num = rand() % 4;
		r.order = order_num;
		//Get random type (Fleet or Army)
		r.type = rand() % 2;
		//Get random home country
		r.country = rand() % 48;
		//Set to country and support country based on order
		int tc = rand() % 48;
		int sc = rand() % 48;
		if (order_num == 0) {
			//hold
			r.tcountry = -1;
			r.scountry = -1;
		} else if (order_num == 1) {
  			//move
			r.tcountry = tc;
			r.scountry = -1;
		} else if (order_num == 2) {
			//support
			r.tcountry = tc;
			r.scountry = sc;
		} else if (order_num == 3) {
			//convoy
			r.tcountry = tc;
			r.scountry = sc;
		}
		//Print output
		/*
		printf("%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",i,r.player,r.order,r.type,r.country);
		printf("To Country - %i, Support Country - %i.\r\n",r.tcountry,r.scountry);
		*/

     	int rc = write(fd, &r, sizeof(r));
  		if (rc != sizeof(r)) {
      	perror("write");
     		return 1;
  		}
   }
   // ok to ignore error code here, because we're done anyhow...
   (void) close(fd);
   free(ptr);
   return 0;
}
