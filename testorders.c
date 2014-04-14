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
            r.tcountry = -1;
            r.scountry = -1;
            break;
        case 1 : //move
            nc = g[country].ncountrys;
            while(a != -1) {
                a = nc[i];
                i++;
            }
            int m = rand() % i;
            r.tcountry = nc[m];
            r.scountry = -1;
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
                r.tcountry = -1;
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

//allows user to enter custom commands for each round
int makeOrders() {
    int stop = 0;
    int num = 0;
    int t;
	printf("Choosing your own orders..\r\n");
    ptr = (int *) malloc(sizeof(order_t));
    order_t *temp = (order_t*) ptr; 
    while (stop == 0) {
        printf("Enter order #%i:\r\n",num);
        printf("player= ");
        scanf("%i", &t);
        temp->player = t;
        printf("order= ");
        scanf("%i", &t);
        temp->order = t;
        printf("type= ");
        scanf("%i", &t);
        temp->type = t;
        printf("country= ");
        scanf("%i", &t);
        temp->country = t;
        if (temp->order != 0) { 
            printf("tcountry= ");
            scanf("%i", &t);
            temp->tcountry = t;
            if (temp->order != 1) { 
                printf("scountry= ");
                scanf("%i", &t);
                temp->scountry = t;
            } else {
                temp->scountry = -1;
            }
        } else {
            temp->tcountry = -1;
        }
        printf("Enter 0 to continue");
        scanf("%i", &t);
        stop = t;
        printf("\r\n");
        temp->valid = 0;
        temp->confirmed = 0;
        o[num] = *temp;
        num++;
   }
   free(ptr);
   return num;
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
