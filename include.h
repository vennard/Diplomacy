#ifndef __include_h__
#define __include_h__

#define MAX_ORDERS 20000

extern region_t g[48];
extern order_t o[MAX_ORDERS];
extern int validOrders;
extern int numO,Sneeded,Rneeded;

extern int loadgamedata(char f[]);
extern int startnewgame(int num);
extern int getTestOrders(int seed,int numOrders, char f[]);
extern int firstvalidate(void);
extern int secondvalidate(void);

//calls for spi communication
extern void runspi(void*);
extern void tx_phase_start(int phasetype, void* ptr);
extern void* rx_orders_start(int roundtype);
extern void configure(void);

//calls for led control
extern void examplegame(void);
extern void writeled(int val, int lednum);
extern void writeout(void);

//calls for timer control
extern void settmr(int min);
extern void starttmr(void);
extern int checktmr(void);


#endif
