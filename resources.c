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

region_t g[48];
order_t o[MAX_ORDERS];
int validOrders = 0;
int numO = 0;

int loadgamedata(char f[]) {
   int fd = open(f, O_RDONLY);
   if (fd < 0) {
  		perror("read");
  		exit(1);
   }
	if (read(fd,g,sizeof(g)) < 0) perror("read");
	printf("Read success!\r\n");
	return 0;
}

int startnewgame(int num) {
  	//just handling one save game -- case statement for more
	printf("Starting new game...");
	char fname[] = "/tmp/savegame";
	if (num == 0) {
	  //fname = "/tmp/savegame1";
	} else {
		return 1;	
	}
	printf(" saved as: %s\r\n",fname);
	genStart(fname); //MUST BE CHANGED TO PERMANENT LOCATION
	loadgamedata(fname);
	return 0;
}

//Print all game data
int printgame() {
    int k;
    for (k = 0;k < 48;k++) printf("-");
    printf("\r\n");
    for (k = 47;k >= 0;k--) {
        printf("%i ",k);
        switch (g[k].type) { 
            case 0 : printf("(inland) "); break;
            case 1 : printf("(coastal) "); break;
            case 2 : printf("(water) "); break;
            default : break;
        }
        if (g[k].supply == 1) printf("+supply ");
        switch (g[k].occupy_type) {
            case -1 : printf("is not occupied.\r\n"); break;
            case 0 : printf("is occupied by a troop "); break;  
            case 1 : printf("is occupied by a fleet "); break;
            default : break;
        }
        switch (g[k].player) {
            case -1 : break;
            case 0 : printf("from Poland.\r\n"); break;  
            case 1 : printf("from Russia.\r\n"); break;
            case 2 : printf("from Sweden.\r\n"); break;
            case 3 : printf("from Finland.\r\n"); break;
            default : break;
        }

    }
}
  
//returns number of orders
int getTestOrders(int seed, int numOrders, char f[]) {
	genOrders(seed,numOrders,f);
   int fd1 = open(f, O_RDONLY);
   if (fd1 < 0) {
  		perror("read");
		return -1;
   }
	//get size of file
	struct stat st;
	fstat(fd1,&st);
	int size = st.st_size;
	numOrders = size / sizeof(order_t);
	if (read(fd1, o, size) < 0) {
	  printf("Reading orders failed!\r\n");
	  return -1;
	}
	return numOrders;
}

int isneighbor(int c, int *nc) {
    int k = 0;
    int valid = 0;
	while (nc[k] != -1) {
	   if (nc[k] == c) valid = 1;  	
		k++;
	}
    return valid;
}

//c - country of origin
//tc - goal landing region for troop convoying
//nc - neighboring countrys of c
//ex - exclude country on search (used for recursive call)
//     should be -1 for original call
int valid = 0;
int checkconvoy(int c, int tc, int *nc,int ex) {
    int k = 0;
    int count = 0;
    int nt[12];
    while (valid == 0) {
        while(nc[k] != -1) {
            int t = nc[k];
            int type = g[t].type;
            int occupy_type = g[t].occupy_type;
            if ((type == 2)&&(occupy_type == 1)&&(ex != t)) {
                nt[count] = t;
                count++;
            }
            //check if you have found target country
            if (tc == t) valid = 1;
            k++;
        }
        //if you have not found tc and there are paths left
        //recursively call this function until no paths are left
        while (count > 0) { 
            int t2 = nt[(count-1)];
            count--;
            checkconvoy(t2,tc,g[t2].ncountrys,c);
        }
    }
    return valid;
}

//First Run through validation check
//TODO must add siljan as ONLY coastal to Van!
int firstvalidate(void) {
	//Check if order is valid
	int i;
	printf("Start of 1st Round validation with %i orders!...\r\n\r\n",numO);
	for (i = 0;i < numO;i++) {
	  	int p = o[i].player;
	  	int c = o[i].country;
		int t = o[i].type;
		int tc = o[i].tcountry;
		int sc = o[i].scountry;
        int or = o[i].order;
	  	if (g[c].player != p) continue; //Player doesn't match 
        if (g[c].occupy_type == -1) continue; //Region doesn't have unit
        switch (or) {
            case 0 : //hold
                validOrders++;
                o[i].valid = 1;
                g[c].dS++;
                continue;
                break;
            case 1 : //move
                if ((g[c].occupy_type == 0)&&(g[tc].type == 2)) continue;
                if ((g[c].occupy_type == 1)&&(g[tc].type == 0)) continue;
                //check move exceptions
                switch (o[i].country) {
                    case 42 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 30)) continue;
                        break;
                    case 46 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 38)) continue;
                        break;
                    case 40 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 38)) continue;
                        break;
                    case 39 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 36)) continue;
                        if ((o[i].type == 1)&&(o[i].tcountry == 38)) continue;
                        break;
                    case 37 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 18)) continue;
                        break;
                    case 34 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 38)) continue;
                        break;
                    case 31 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 30)) continue;
                        break;
                    case 30 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 42)) continue;
                        if ((o[i].type == 1)&&(o[i].tcountry == 31)) continue;
                        if ((o[i].type == 1)&&(o[i].tcountry == 25)) continue;
                        break;
                    case 25 : //TODO add support for multiple coasts
                        if ((o[i].type == 1)&&(o[i].tcountry == 30)) continue;
                        break;
                    case 17 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 9)) continue;
                        break;
                    case 9 :
                        if ((o[i].type == 1)&&(o[i].tcountry == 17)) continue;
                        break;
                }
                if (!isneighbor(tc,g[c].ncountrys)) continue;
	  		        printf("#%i move | ",i);
                    validOrders++;
                    o[i].valid = 1;
                    g[tc].aS++; //++attack strength to target country
                break;
            case 2 : //support
                //check correct target country types
                if ((t == 0)&&(g[tc].type == 2)) continue;  
                if ((t == 1)&&(g[tc].type == 0)) continue;
                if (g[sc].occupy_type == -1) continue; 
                if (!isneighbor(sc,g[c].ncountrys)) continue; 
                if (tc == -1) { //supporting a hold,convoy, or support
	  		        printf("#%i support hold | ",i);
                    o[i].valid = 1;
                    g[sc].dS++; //++defense strength of support country
                    validOrders++;
                    continue;
                 } else { //supporting a move
                    //sc and tc must be neighbors of eachother and c
                    if (!isneighbor(tc,g[sc].ncountrys)) continue;
                    if (!isneighbor(tc,g[c].ncountrys)) continue;
	  		        printf("#%i support move | ",i);
                    o[i].valid = 1;
                    g[tc].aS++; //++attack strength to target country
                    validOrders++;
                    continue;
                }
                break;
            case 3 : //convoy
                if (g[c].occupy_type == 0) { //land units convoy order
                    if (g[c].type != 1) continue; //region must be coastal
                    //land unit must be neighbor of fleet & a fleet must exist
                    if ((!isneighbor(c, g[sc].ncountrys))||(g[sc].occupy_type != 1)) continue; 
                    //check path for valid fleets to target country
                    if (checkconvoy(c,tc,g[c].ncountrys,-1) != 1) continue;
	  		        printf("#%i troop convoy | ",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                } else if (g[c].occupy_type == 1) { //fleets convoy order
                    if (g[c].type != 2) continue; //region must be water
                    if (g[sc].type != 1) continue; //troop must be coastal
                    if (g[sc].occupy_type != 0) continue; //troop must exist
                    //search for fleet path to troop and target country
                    if (checkconvoy(c,tc,g[c].ncountrys,-1) != 1) continue;
                    if (checkconvoy(c,sc,g[c].ncountrys,-1) != 1) continue;
	  		        printf("#%i fleet convoy | ",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                } else {
                    continue;
                }
                break;
            default :
                break;
        }

    }


	printf("\r\n\r\nNumber of valid orders after 1st validation = %i\r\n",validOrders);
	int j;
	for(j = 0;j < numO;j++) {
	  if (o[j].valid == 1) {
		printf("Order #%i: Player - %i, Order - %i, Army/Fleet - %i, Country - %i, ",j,o[j].player,o[j].order,o[j].type,o[j].country);
		printf("To Country - %i, Support Country - %i.\r\n",o[j].tcountry,o[j].scountry);
	  }
	}
	return validOrders;
}

int moveunit(int c, int tc) {
    g[tc].player = g[c].player;
    g[tc].occupy_type = g[c].occupy_type; 
    g[c].player = -1;
    g[c].occupy_type = -1;
    return 0;
}
int secondvalidate() {
    printf("Starting second round validation...");
    int vo[255]; //valid orders
    int count = 0;
    //find all valid orders
    int k,j;
    for(k = 0;k < numO;k++) {
        if(o[k].valid == 1) {
            vo[count] = k;
            count++;

        }
    }
    printf(" found %i valid orders!\r\n",count);
    //check over valid orders
    order_t to;
    for(k = 0;k < count;k++) {
        to = o[vo[k]];
        int a,d;
        int c = to.country;
        int tc = to.tcountry;
        int sc = to.scountry;
        switch(to.order) {
            case 0 : //hold
                d = g[c].dS;
                a = g[c].aS;
                printf("Analysing a hold order... defense=%i vs attack=%i... ",d,a);
                if (d > a) {
                    printf("success! \r\n");
                    o[vo[k]].confirmed = 1;
                } else if (d == a) {
                    printf("standoff! \r\n");
                } else {
                    //TODO add mark on order for needed resolution on retreat phase
                    printf("failed! \r\n");
                }
                break;
            case 1 : //move
                d = g[tc].dS;
                a = g[tc].aS;
                printf("Analysing a move order... defense=%i vs attack=%i... ",d,a);
                if (a > d) {
                    printf("success! \r\n");
                    o[vo[k]].confirmed = 1;
                } else if (d == a) {
                    printf("standoff! \r\n");
                } else {
                    printf("failed! \r\n");
                }
                break;
            case 2 : //support
                d = g[c].dS;
                a = g[c].aS;
                printf("Analysing a support order... defense=%i vs attack=%i... ",d,a);
                if (d > a) {
                    printf("success! \r\n");
                    o[vo[k]].confirmed = 1;
                } else if (d == a) { //still provides support TODO check
                    printf("standoff! \r\n");
                } else { 
                    printf("failed! \r\n");
                    //TODO add remove this support defense bonus to sc
                }

                break;
            case 3 : //convoy TODO
                printf("Analysing a convoy order... defense=%i vs attack=%i... ",d,a);
                printf(" coming soon.\r\n");
                break;
            default :
                break;
        }
    }
    //TODO add conflict resolution code (ie two units moving to same country, supports being cut, etc)
    int a1, a2;
    for(k = 0;k < numO;k++) {
        if (o[k].valid == 0) continue;
        if (o[k].order == 1) { //find conflicting moves
            int tc = o[k].tcountry;
            for(j = 0;j < numO;j++) {
                if (j == k) break;
                if (o[j].valid == 0) continue;
                if ((o[j].order == 1)&&(o[j].tcountry == tc)) {
                    //found conflict -- now check strengths
                    printf("Found a move conflict at order #%i and #%i!! removing...\r\n",k,j);
                    a1 = g[o[k].country].aS;
                    a2 = g[o[j].country].aS;
                    if (a1 == a2) { //evenly matched move -- both fail
                        //mark one as invalid and one as confirmed = -1
                        //this allows for further conflicts to be found
                        o[k].valid = 0;
                        o[j].confirmed = -1;
                    } else if (a1 > a2) { //move 1 wins
                        o[j].valid = 0;
                    } else if (a2 > a1) {
                        o[k].valid = 0;
                    }
                }
            }
        }
    }
    //TODO remove all orders marked with confirm = -1
    for(k = 0;k < numO;k++) {
        if (o[k].confirmed == -1) o[k].valid == 0;
    }

    //modify game board with confirmed orders
    printf("\r\n");
    for(k = 0;k < numO;k++) {
        if(o[k].confirmed == 1) {
            order_t co = o[k];
            switch (co.order) {
                case 0 : 
                    printf(" HOLD P%i U%i C%i |",co.player,co.type,co.country);
                    break;
                case 1 :
                    printf(" MOVE P%i U%i C%i -> TC%i |",co.player,co.type,co.country,co.tcountry);
                    moveunit(co.country, co.tcountry);
                    break;
                case 2 :
                    printf(" SUPPORT P%i U%i C%i +> (SC%i -> TC%i) |",co.player,co.type,co.country,co.scountry,co.tcountry);
                    break;
                case 3 :
                    printf(" CONVOY P%i U%i C%i +> (SC%i -> TC%i) |",co.player,co.type,co.country,co.scountry,co.tcountry);
                    break;
                default :
                    break;
            }
        }
    }
    printf("\r\n\r\n");
    return 0;
}

//find and remove duplicate orders -- last valid order in wins
int removeduplicates(){
    printf("Checking for duplicate orders...\r\n");
    int k;
    int j;
    for(k = 0;k < numO;k++) {
        if (o[k].valid == 0) continue;
        for(j = 0;j < numO;j++) {
            if (o[j].valid == 0) continue;
            if (j == k) continue;
            if ((o[k].player == o[j].player)&&(o[k].country == o[j].country)) {
                //mark previous order as invalid
                o[k].valid = 0;
                printf("removed duplicate order! #%i - %i player %i vs. #%i - %i player %i\r\n",k,o[k].country,o[k].player,j,o[j].country,o[j].player);
                validOrders--;
            }
        }
        

    }
    return 0;
}

//clean up game board (remove round stats)
int clean(){
    int k;
    for(k = 0;k < 48;k++) {
        g[k].dS = 0;
        g[k].aS = 0;
    }
    return 0;
}


