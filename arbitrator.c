nclude <stdio.h>
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
void
usage(char *prog)
{
   fprintf(stderr, "usage: %s <-i input game data file> <-o input orders file>\n", prog);
   exit(1);
}

int
main(int argc, char *argv[])
{

// Setup Board

// DEBUG CODE (INPUT FILE ORDERS)
   // arguments
   char *inFile   = "/no/such/file";
char *inOrders = "/no/such/thing";

   // input params
   int inparms;
   opterr = 0;
   while ((inparms = getopt(argc, argv, "i:o:")) != -1) {
switch (inparms) {
case 'i':
   inFile = strdup(optarg);
   break;
case 'o':
   inOrders = strdup(optarg);
   break;
default:
   usage(argv[0]);
}
  }

   // open game start data file
   int fd = open(inFile, O_RDONLY);
   if (fd < 0) {
  perror("read");
  exit(1);
   }

// open game orders file
   int fd1 = open(inOrders, O_RDONLY);
   if (fd1 < 0) {
  perror("read");
  exit(1);
   }

//Get size of game start data file
struct stat st;
int num_regions = 0;
stat(inFile, &st);
int size_infile = st.st_size; //total size of file in bytes
num_regions = size_infile / sizeof(region_t);
//Get size of incoming orders file TODO
/*
struct stat st;
int num_orders = 0;
stat(inFile, &st);
int size_infile = st.st_size; //total size of file in bytes
num_orders = size_infile / sizeof(order_t);
*/

//Create and allocate memory for game start data
struct region_t regions[48];
region_t *rptr = regions;
rptr = (region_t *) malloc(sizeof(region_t)*num_regions);
//Create and allocate memory for incoming orders TODO

   //Read in game start data
region_t r;
int index = 0;
   while(1) {
     int rc;
     rc = read(fd, &r, sizeof(r));
     if (rc == 0) break; //EOF
     if (rc < 0) {
         perror("read");
         exit(1);
     }

//DEBUG OUTPUT
     printf("Country %i: ",r.name);
if (r.type == 0) printf(" Inland, ");
//ENDED HERE
printf(" Type - %i",r.type);
printf(" Occupy Type - %i",r.);
printf(" Country: %i",t.country);
printf(" tCountry: %i",t.tcountry);
printf(" sCountry: %i\r\n",t.scountry);

//Save input into local memory
in_order[index] = t;
index++;
   }
// END DEBUG CODE (INPUT FILE ORDERS)

//Start of True Arbitrator Code
int a = 0;
//for(a = 0;a < num_orders;a++) {
order_t order = in_order[a];
int p = order.player;
int o = order.order;
int type = order.type;
int c = order.country;
int tc = order.tcountry;
int sc = order.scountry;
printf("CURRENT ORDER: ");
printf(" Player: %i",p);
printf(" Order: %i",o);
printf(" Type: %i",type);
printf(" Country: %i",c);
printf(" tCountry: %i",tc);
printf(" sCountry: %i\r\n",sc);

// check if its an invalid order
// transform all invalid orders into holds
// Analyse country by country to see results
// Store transitions for display

//}

   
   // ok to ignore error code here, because we're done anyhow...
   (void) close(fd);
free(dptr);
   return 0;
}
