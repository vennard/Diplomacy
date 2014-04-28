#ifndef __include_h__
#define __include_h__

#define MAX_ORDERS 20000

extern region_t g[48];
extern order_t o[MAX_ORDERS];
extern int validOrders;
extern int numO,Sneeded,Rneeded;

int loadgamedata(char f[]);
int startnewgame(int num);
int getTestOrders(int seed,int numOrders, char f[]);
int firstvalidate(void);
int secondvalidate(void);

//calls for spi communication
void runspi(void);
void tx_phase_start(int phasetype, void* ptr);
int rx_orders_start(int roundtype);
void configurespi(void);

//calls for led control
void examplegame(void);
void writeled(int val, int lednum);
void writeout(void);

//calls for timer control
void settmr(int min);
void starttmr(void);
int checktmr(void);

int makeOrders();
int printgame();
int arbitor();
int supplycheck();
int supplyphase();
void testfunc();
int removeduplicates();
int insertholds();
int evaluate();
int validate();
int execute();
int clean();
#endif

