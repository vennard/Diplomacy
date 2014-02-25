#ifndef __order_h__
#define __order_h__

typedef struct __order_t {
		  int player;
		  int order;
		  int type; //0 - army 1 - fleet
		  int country; //players country -- see below for table
		  int tcountry; // destination country for move or support (this is support players destination)
		  int scountry; //only valid for support orders - home of support players unit
		  						 //-1 if not a support order
} order_t;

#endif
