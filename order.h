#ifndef __order_h__
#define __order_h__

typedef struct __order_t {
  		  int valid; // TO BE SET BY ARBITRATOR
        int confirmed; // TO BE SET BY ARBITRATOR
		  int player; //0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland
		  int order; // 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
		  int type; //0 - army 1 - fleet
		  int country; //players country -- see google drive for mapping
		  int tcountry; 
          // for move: destination country
          // for support move: destination country of supported move
          // for convoy: destination of troop
		  int scountry; 
          // for support hold: supported units country
          // for support move: supported units origin country
          // for troop issued convoy: -1
} order_t;

int genOrders(int seed,int numOrders, char outfile[]);

#endif
