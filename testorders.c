#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "order.h" //see file for definitions
//Creating sample output from the controllers
// Sending string (bytes)
// Player Num (1) / Order (1) / Unit Location (3) / [To Country (3)]
//
// Player Num = 1 -> 4
// Orders: 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
// BY DEFAULT -1 represents no country (invalid)

int genOrders(int seed, int numOrders, char out[]) {
   char *outFile   = "/tmp/outfile";
   int ordersLeft = 0;
	int sd = 0;
   int* ptr;
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
