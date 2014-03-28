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
order_t r;

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
int iorder(int s) {
    srand(s);
    int rnd = rand();
    int a = 0, b = 0, c = 0;
    int movetype;
    int *nc;
    int *tc;
    int temp = 0;
    int temp1 = 0;
    int temp2 = 0;
    int i = 0;
    int found = 0;
    int hasMatch = 0;
    while (found == 0) {
        r.player = rand() % 4;
        r.order = rand() % 4;
        r.type = rand() % 2;
        found = findunits(r.player, r.type);
    }
    //printf("found %i valid units for player %i of type %i \r\n",found, r.player,r.type);
    int x = rnd % found;
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
            movetype = rand() % 2;
            if (movetype == 0) { //do defensive support
                nc = g[country].ncountrys;
                while(temp != -1) {
                    temp = nc[a];
                    if (g[temp].occupy_type!=-1) r.scountry = temp;
                    a++;
                }
            } else { //attacking support
                nc = g[country].ncountrys;
                temp = 0;
                temp1 = 0;
                while (temp != -1) {
                    temp = nc[a];
                    if (g[temp].occupy_type!=-1) { //found occupied n country to support
                        tc = g[temp].ncountrys;
                        while (temp1 != -1) { //search its  neighbors to make sure its a neighbor as well
                            temp1 = tc[b];
                            while (temp2 != -1) {
                                temp2 = nc[c];
                                if (temp1 == temp2) { //found matching target country
                                    r.tcountry = temp1;
                                    r.scountry = temp;
                                }
                                c++;
                            }
                            b++;
                        }
                    }
                    a++;
                }
            }
             break;
        case 3 : //convoy
             nc = g[country].ncountrys;
             //choose correct support region type 
             if (r.type == 0) { //army
                 while(temp != -1) {
                    temp = nc[a];
                    if (g[temp].type==2) {
                        r.scountry = temp; 
                        temp = -1;
                    }
                    a++;
                 }
                 if (r.scountry == -1) r.scountry = rand() % 48;
              } else { //fleet
                    while(temp != -1) {
                       temp = nc[a];
                       if (g[temp].type==1) {
                           r.scountry = temp; 
                           temp = -1;
                       }
                       a++;
                    }
                if (r.scountry == -1) r.scountry = rand() % 48;
                }
                //choose correct target type and region
                a = 0;
                temp = r.scountry;
                tc = g[temp].ncountrys;
                while(temp != -1) {
                    temp = nc[a];
                    if (g[temp].type == 1) {
                        r.tcountry = temp;
                        temp = -1;
                    }
                    a++;
                }
                r.tcountry = rand() % 48;
                break;
        default :
             break;
        }
    return 0;
}

int genOrders(int seed, int numOrders, char out[]) {
    ptr = (int *) malloc(sizeof(r));
	printf("Generating %i Orders..\r\n",numOrders);

	if (strlen(out) < 1) {
		printf("No outfile set, resorting to default /tmp/outfile\r\n");
	} else {
	  outFile = out;
   }
	if (seed < 0) {
		printf("Seed set was negative! Setting to 1.. \r\n");
        seed = 1;
	} 
	if (numOrders < 1) {
		printf("No orders set... thats not very adventurous - exiting\r\n");
		return 0;
	} else {
	  ordersLeft = numOrders;
   }

   // open and create output file
   int fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
   if (fd < 0) {
  		perror("open");
  		return 1;
   }

   int i;
   for (i = 0; i < ordersLeft; i++) {
/*
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
        */
		//Print output
		/*
		printf("%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",i,r.player,r.order,r.type,r.country);
		printf("To Country - %i, Support Country - %i.\r\n",r.tcountry,r.scountry);
		*/
        iorder(seed);
        seed += 3;
        seed = seed * 2;
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
