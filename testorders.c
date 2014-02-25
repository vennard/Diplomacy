nclude <stdio.h>
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

//Legacy -- Replacing with integers [0-47]
char* orders[] = {"ode", "str", "cop", "kat", "ska", "vis", "dan", "sba",
                 "ron", "hab", "mal", "bol", "got", "war", "wkr", "kon",
                 "mba", "sli", "gos", "vat", "bre", "min", "nar", "rig",
  "gor", "saa", "nba", "sto", "sil", "wes", "tal", "gof",
                 "hel", "sgo", "sun", "stj", "keb", "stp", "sai", "kuo",
"vas", "ngo", "tor", "kuy", "oul", "ima","van","lla" };
void
usage(char *prog)
{
   fprintf(stderr, "usage: %s <-s random seed> <-n number of orders> <-o output file>\n", prog);
   exit(1);
}

int
main(int argc, char *argv[])
{
order_t r;
   int* ptr;
   ptr = (int *) malloc(sizeof(r));

   // arguments
   int randomSeed  = 0;
   int ordersLeft = 0;
   char *outFile   = "/no/such/file";

   // input params
   int c;
   opterr = 0;
   while ((c = getopt(argc, argv, "n:s:o:")) != -1) {
switch (c) {
case 'n':
   ordersLeft = atoi(optarg);
   break;
case 's':
   randomSeed  = atoi(optarg);
   break;
case 'o':
   outFile     = strdup(optarg);
   break;
default:
   usage(argv[0]);
}
   }

   // seed random number generator
   srand(randomSeed);

   // open and create output file
   int fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
   if (fd < 0) {
  perror("open");
  exit(1);
   }

   int i;
   for (i = 0; i < ordersLeft; i++) {
     //Get random player
     r.player = rand() % 4;
     //Get random order
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
}else if (order_num == 1) {
  //move
r.tcountry = tc;
r.scountry = -1;
}else if (order_num == 2) {
//support
r.tcountry = tc;
r.scountry = sc;
}else if (order_num == 3) {
//convoy
r.tcountry = tc;
r.scountry = sc;
}
//Print output
printf("%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",i,r.player,r.order,r.type,r.country);
printf("To Country - %i, Support Country - %i.\r\n",r.tcountry,r.scountry);


     int rc = write(fd, &r, sizeof(r));
  if (rc != sizeof(r)) {
     perror("write");
     exit(1);
  }
   }


   
   // ok to ignore error code here, because we're done anyhow...
   (void) close(fd);

   free(ptr);
   return 0;
}
