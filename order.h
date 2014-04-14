#ifndef __order_h__
#define __order_h__

//TODO add support for adding unit orders
typedef struct __order_t {
		  int player; //0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland
		  int order; // 0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
		  int type; //0 - army 1 - fleet
		  int country; //players country -- see google drive for mapping
		  int tcountry; 
          // for move: destination country
          // for support hold: -1
          // for support move: destination country of supported move
          // for convoy army & fleet: destination of army 
		  int scountry; 
          // for support hold: supported units country
          // for support move: supported units origin country
          // for convoy army: -1 TODO check this is implemented
          // for convoy fleet: troop to move
  		  int valid; // TO BE SET BY ARBITRATOR
          int confirmed; // TO BE SET BY ARBITRATOR
} order_t;

int genOrders(int seed,int numOrders, char outfile[]);

#endif
