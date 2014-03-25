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

#endif
