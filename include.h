#ifndef __include_h__
#define __include_h__

#define MAX_ORDERS 20000
#define ORDERS_TIME 15
#define RETREAT_TIME 5
#define SUPPLY_TIME 5

extern region_t g[48];
extern order_t o[MAX_ORDERS];
extern int validOrders;
extern int numO,Sneeded,Rneeded,numinc;

int arbitor(void);
int loadgamedata(char f[]);
int startnewgame(int num);
int getTestOrders(int seed,int numOrders, char f[]);
int firstvalidate(void);
int secondvalidate(void);

//calls for spi communication
void runspi(void);
void tx_phase_start(int phasetype);
int rx_orders_start(int roundtype);
void configurespi(void);
void demo(void);
void testcontroller(void);

//calls for led control
void examplegame(void);
void writeled(int val, int lednum);
void writeout(void);
void writeregion(int region);
void fancystart(void);
void logo(void);
void clearboard(void);

//calls for timer control
void settmr(int min);
void starttmr(void);
char checktmr(void);

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

