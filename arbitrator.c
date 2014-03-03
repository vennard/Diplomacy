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

//Creating sample output from the controllers
// Sending string (bytes)
// Player Num (1) / Order (1) / Unit Location (3) / [To Country (3)]
//
// Player Num = 1 -> 4
// Orders: 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy // BY DEFAULT 000 = NULL (represents no country)

int numValidOrders = 0;
int arbitor() {
	printf("Starting the Arbitrator...\r\n");
    //first round is simple exclusion for incorrect orders
    //also counts up variables used for second validate
    numValidOrders = firstvalidate();
	if (numValidOrders == -1) perror("first validate failed");
    //throw out duplicate / outdated orders (ie player gives two orders to the same unit)
    removeduplicates();
    //second validation handles contention
    numValidOrders = secondvalidate();
    //then move game board according to confirmed orders and output

    //clean game board stats used by arbitor
    clean();
   validOrders = 0;
   return 0;
}
