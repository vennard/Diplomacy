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

int numValidOrders = 0;

int arbitor() {
	printf("Starting the Arbitrator...\r\n");
    removeduplicates();
    numValidOrders = firstvalidate();
	if (numValidOrders == -1) perror("first validate failed");

    validate(2);
    validate(1);
    validate(3);
    validate(0);

    execute();
    clean();
    return 0;
}

/* Order Validation process
 * 1. firstvalidate: makes first run check to determine if order location, unit, unit type, and targets are correct
 * 2. removeduplicates: removes all outdated orders
 * 3. validate: runs through confirming all orders - tries to invalidate each order (looks for two units moving to the same place or cutoffs)
 * 4. resolve: looks at valid but unconfirmed orders - adjusts strengths and rerun validate 
 * 5. execute: runs valid orders once they are all confirmed
 * 6. clean: cleans all used variables (board strengths and round counts etc)
 */
