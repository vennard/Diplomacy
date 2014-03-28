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
        if (o[i].valid == -1) continue; //marked as invalid duplicate 
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
                    g[c].aS++; //++attack strength to country
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
                    g[c].dS++; //supporting country has own strength if attacked
                    validOrders++;
                    continue;
                 } else { //supporting a move
                    //sc and tc must be neighbors of eachother and c
                    if (!isneighbor(tc,g[sc].ncountrys)) continue;
                    if (!isneighbor(tc,g[c].ncountrys)) continue;
	  		        printf("#%i support move | ",i);
                    o[i].valid = 1;
                    g[sc].aS++; //++attack strength to support country
                    g[c].dS++; //supporting country has own strength if attacked
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
                    g[c].aS++;
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

//modify game board with confirmed orders
int execute() {
    int k;
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
                    //TODO if troop moveunit(co.country, co.tcountry) 
                    break;
                default :
                    break;
            }
        }
    }
    printf("\r\n\r\n");
    return 0;
}

//Analyses orders for conflicts
//Can be rerun if strengths are changed
//type = order type to be analyzed
int validate(int type) {
    int vo[255]; //valid orders
    int count = 0;
    int k,j;
    //find all valid orders
    for(k = 0;k < numO;k++) {
        if(o[k].valid == 1) {
            vo[count] = k;
            count++;
        }
    }
    order_t to,ro;
    for(k = 0;k < count;k++) {
        to = o[vo[k]];
        if (to.order != type) continue; //skip entries of incorrect order type
        int a,d,temp;
        int c = to.country;
        int tc = to.tcountry;
        int sc = to.scountry;
        int check = 1;
        switch(to.order) {
            case 0 : //hold
                d = g[c].dS;
                printf("Reviewed hold order #%i with dS=%i... ",vo[k],d);
                //find any moves that have higher attack
                int check = 1;
                for (j = 0;j < count;j++) {
                    if (j == k) continue;
                    ro = o[vo[j]];     
                    if (ro.tcountry == c) {
                        if (g[ro.country].aS > d) { 
                            check = 0;
                            o[vo[k]].confirmed = -2; //mark order for resolution
                            Rneeded = 1;
                            printf("failed by order #%i with aS=%i. \r\n",vo[j],g[ro.country].aS);
                        }
                    }
                }
                if (check) {
                    o[vo[k]].confirmed = 1; //If no higher strength move then confirm
                    printf("success! \r\n");
                }
                break;
            case 1 : //move -- check for standoff moves (2 units to the same space)
                d = g[tc].dS;
                a = g[c].aS;
                check = 1;
                for (j = 0;j < count;j++) {
                    ro = o[vo[j]];     
                    if (j == k) continue;
                    //invalidate if found move order with >= strength moving to same country
                    if ((ro.order == 1)&&(ro.tcountry==tc)&&(g[ro.country].aS == a)) check = -1;
                    if ((ro.order == 1)&&(ro.tcountry==tc)&&(g[ro.country].aS > a)) check = 0;
                }
                printf("Analysed move order #%i aS=%i against dS=%i... ",vo[k],a,d);
                if (check == 1) {
                    if (a > d) {
                        printf("success! \r\n");
                        o[vo[k]].confirmed = 1;
                    }
                } else if (check == 0){
                    printf("found another move with greater strength!\r\n");
                    o[vo[k]].valid = 0;
                    o[vo[k]].confirmed = -2; //mark for retreat resolution
                    Rneeded = 1;
                } else {
                    printf("standoff with another unit! \r\n");
                    o[vo[k]].valid = 0;
                    o[vo[k]].confirmed = 1; 
                }
                break;
            case 2 : //support 
                //RULE: support is cutoff if country supporting is attacked from any country but the country its supporting
                //the country its supporting can only cut support by dislodging unit (ie aS > dS)
                d = g[c].dS;
                printf("Analysing a support order... defense=%i ",d);
                check = 1;
                for (j = 0;j < count;j++) {
                    if (j == k) continue;
                    ro = o[vo[j]];     
                    a = g[ro.country].aS;
                    if ((ro.order == 1)&&(ro.tcountry == c)) check = 0;
                    if ((ro.order == 1)&&(ro.country == sc)&&(ro.tcountry == c)&&(a > d)) check = -1;
                }
                if (check == 1) {
                    printf("success! \r\n");
                    o[vo[k]].confirmed = 1;
                } else if (check == 0) {
                    printf("support cutoff by move! invalidating... \r\n");
                    o[vo[k]].valid = 0;
                    o[vo[k]].confirmed = 1;
                    //remove support strengths
                    if (ro.tcountry != -1) g[ro.scountry].aS--;
                    if (ro.tcountry == -1) g[ro.scountry].dS--;
                } else {
                    printf("support dislodged by supported country move with greater strength! \r\n");
                    o[vo[k]].valid = 0;
                    //remove support strengths
                    if (ro.tcountry != -1) g[ro.scountry].aS--;
                    if (ro.tcountry == -1) g[ro.scountry].dS--;
                    //Mark for retreat resolution
                    o[vo[k]].confirmed = -2; //mark order for resolution
                    Rneeded = 1;
                }
                break;
            case 3 : //convoy TODO
                //RULE 1: dislodgment of a fleet in a convoy can cause convoy to fail
                //RULE 2: if convoyed army would standoff at destination, convoy fails
                if (g[c].occupy_type == 0) { //army convoy order
                    printf("validating army convoy order... ");
                    //look for RULE 2
                    d = g[c].aS; //TODO figure out how to add support strength to convoy troops
                    check = 1;
                    for (j = 0;j < count;j++) {
                       if (j == k) continue; 
                       ro = o[vo[j]];     
                       a = g[ro.country].aS;
                       if ((ro.tcountry == tc)&&(a == d)&&(ro.type == 1)) {
                           temp = ro.country;
                           check = -1;  //found conflicting move
                       }
                       if ((ro.tcountry == tc)&&(a > d)&&(ro.type == 1)) {
                           temp = ro.country;
                           check = -2;//found conflicting move
                       }
                    }
                    if (check == 1) {
                        printf("success! \r\n");
                        o[vo[k]].confirmed = 1;
                    } else if (check == -1) {
                        printf("failed! standoff with c-%i! \r\n",temp);
                        o[vo[k]].confirmed = 1;
                        o[vo[k]].valid = 0; //invalidate
                    } else if (check == -2) {
                        printf("failed! move with greater strength from c-%i! \r\n",temp);
                        o[vo[k]].confirmed = 1;
                        o[vo[k]].valid = 0; //invalidate
                    }
                } else if (g[c].occupy_type == 1) { //fleet convoy order
                    printf("validating fleet convoy order... ");
                    d = g[c].aS; //TODO figure out how to add support strength to convoy troops
                    check = 1;
                    for (j = 0;j < count;j++) {
                        if (j == k) continue;
                        ro = o[vo[j]];     
                        a = g[ro.country].aS;
                        if ((ro.type == 1)&&(a > d)) { //found greater move
                            check = -1;
                            temp = ro.country;
                        }
                    }
                    if (check == 1) {
                        printf("success! \r\n");
                        o[vo[k]].confirmed = 1;
                    } else {
                        printf("failed! was dislodged by c-%i! \r\n",temp);
                        o[vo[k]].confirmed = -2; //mark order for resolution
                        Rneeded = 1;
                        //invalidate convoy order
                        for (j = 0;j < count;j++) {
                            ro = o[vo[j]];     
                            if ((ro.type==3)&&(ro.tcountry==tc)) {
                                o[vo[j]].valid = 0;
                                o[vo[j]].confirmed = 1;
                            }
                        }
                    }
                }
                break;
            default :
                break;
        }
    }
    return 0;
}

//looks at valid but unconfirmed orders
int resolve() {

}

//find and remove duplicate orders -- last valid order in wins
int removeduplicates(){
    printf("Checking for duplicate orders out of %i orders...\r\n", numO);
    int k;
    int j;
    for(k = 0;k < numO;k++) {
        for(j = 0;j < numO;j++) {
            if (j == k) continue;
            if (o[j].valid == -1) continue;
            if ((o[k].player == o[j].player)&&(o[k].country == o[j].country)) {
                //mark previous order as invalid
                o[k].valid = -1;
                printf("removed duplicate #%i (left #%i) player %i at country %i \r\n",k,j,o[k].player,o[k].country);
            }
        }
    }
    return 0;
}

//clean up game board (remove round stats)
int clean(){
    validOrders = 0;
    int k;
    for(k = 0;k < 48;k++) {
        g[k].dS = 0;
        g[k].aS = 0;
    }
    return 0;
}


