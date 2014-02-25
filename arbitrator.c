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
void
usage(char *prog)
{
   fprintf(stderr, "usage: %s <-i input game data file> <-o input orders file>\n", prog);
   exit(1);
}

int
main(int argc, char *argv[])
{
  char tmp[] = { "/tmp/outfile"};
  genOrders(2, tmp);

// Setup Board -- TODO make legit tmp files
//   char *inFile   = "/no/such/file";
//char *inOrders = "/no/such/thing";
/*
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

*/
   return 0;
}
