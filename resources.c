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
        if (g[k].supply != -2) printf("+supply ");
        switch (g[k].occupy_type) {
            case 2 : printf("is not occupied.\r\n"); break;
            case 0 : printf("is occupied by a troop "); break;  
            case 1 : printf("is occupied by a fleet "); break;
            default : break;
        }
        switch (g[k].player) {
            case 4 : break;
            case 0 : printf("from Poland.\r\n"); break;  
            case 1 : printf("from Russia.\r\n"); break;
            case 2 : printf("from Sweden.\r\n"); break;
            case 3 : printf("from Finland.\r\n"); break;
            default : break;
        }

    }
return 0;
}

//checks if supply phase is needed, and what countrys need to remove units
//returns Sneeded
int supplycheck(){
    printf("Checking supply control.\r\n");
    int Sneeded = 0;
    int numunits = 0;
    int numsupplys = 0;
    int i, j, k, old, diff;
    //first check for supply turnovers
    for (k = 0;k < 48;k++) {
        if ((g[k].supply != g[k].player)&&(g[k].occupy_type != 2)) {
            old = g[k].supply;
            g[k].supply = g[k].player;
            printf("Supply turnover from player %i to player %i (country %i)!\r\n", old, g[k].supply, k);
        }
    }
    //check units vs. supplys
    for (i = 0;i < 4;i++) { //loop through players
        for (j = 0;j < 48;j++) { //look for players units
            if ((g[j].player == i)&&(g[j].occupy_type != 2)) numunits++;
            if (g[j].supply == i) numsupplys++;
        }
        diff = numsupplys - numunits; 
        if (diff > 0) {
            Sneeded = 1;
            printf("Player %i gets to gain %i more units!\r\n", i, diff);
        } else if (diff < 0) {
            Sneeded = 1;
            printf("Player %i loses %i units!\r\n", i, -1*diff);
        } else {
            printf("Player %i has equal supply and unit control!\r\n", i);
        }
    }
    return Sneeded;
}

//check supply phase orders -- will place new unit on adjacent square if no order selected TODO
int supplyphase() {
    //check all orders for valid placement of new units / removal
    return 0;
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
//check involves NO CONFLICT RESOLUTION
//TODO must add siljan as ONLY coastal to Van!
int firstvalidate(void) {
   int i;
   printf("Start of 1st Round validation with %i orders!...\r\n\r\n",numO);
   for (i = 0;i < numO;i++) {
       int p = o[i].player;
       int c = o[i].country;
       int t = o[i].type;
       int tc = o[i].tcountry;
       int sc = o[i].scountry;
        int or = o[i].order;
    //    printf("ORDER %i: player - %i, type - %i, order - %i, country - %i, tcountry - %i, scountry - %i\r\n",i,p,t,or,c,tc,sc);
        if (o[i].valid == -1) continue; //marked as invalid duplicate 
       if (g[c].player != p) continue; //Player doesn't match 
        if (g[c].occupy_type == 2) continue; //Region doesn't have unit
        switch (or) {
            case 0 : //hold
                validOrders++;
                o[i].valid = 1;
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
                break;
            case 2 : //support
                //check correct target country types
                if (tc != -1) {
                    if ((t == 0)&&(g[tc].type == 2)) continue;  
                    if ((t == 1)&&(g[tc].type == 0)) continue;
                }
                if ((t == 0)&&(g[sc].type == 2)) continue;
                if ((t == 1)&&(g[sc].type == 0)) continue;
                if (g[sc].occupy_type == 2) continue; 
                if (!isneighbor(sc,g[c].ncountrys)) continue; 
                if (tc == -1) { //supporting a hold, convoy, or support
                   printf("#%i support hold | ",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                 } else { //supporting a move
                    //sc and tc must be neighbors of eachother and c
                    //if (!isneighbor(tc,g[sc].ncountrys)) continue;
                    if (!isneighbor(tc,g[c].ncountrys)) continue;
                    if (!isneighbor(sc,g[c].ncountrys)) { //support country not neighbor - check for support convoy
                        int l;
                        int chk = 0;
                        for (l = 0;l < numO;l++) {
                            if ((o[l].country == sc)&&(o[l].order == 3)) chk = 1; 
                        }
                        if (chk == 0) continue;
                    }
                   printf("#%i support move | ",i);
                    o[i].valid = 1;
                    validOrders++;
                    continue;
                }
                break;
            case 3 : //convoy
                if (g[c].occupy_type == 0) { //army convoy order
                    if (g[c].type != 1) continue; //region must be coastal
                    //land unit must be neighbor of fleet & a fleet must exist TODO double check this rule
                    //if ((!isneighbor(c, g[sc].ncountrys))||(g[sc].occupy_type != 1)) continue; 
                    if (sc != -1) continue;
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
    g[c].player = 4;
    g[c].occupy_type = 2;
    return 0;
}

//modify game board with confirmed orders
int execute() {
    int k;
    printf("\r\n");
    for(k = 0;k < numO;k++) {
        //if((o[k].confirmed == 1)&&(o[k].valid == 1)) {
        if(o[k].valid == 1) {
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
                    if (co.type == 0) { //troop convoy order
                        moveunit(co.country,co.tcountry);
                    }
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
    printf("type = %i, and num valid orders = %i!\r\n",type,count);
    for(k = 0;k < count;k++) {
        to = o[vo[k]];
        if (to.order != type) continue; //skip entries of incorrect order type
        int a,d,temp;
        int c = to.country;
        int tc = to.tcountry;
        int sc = to.scountry;
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
                printf("Enter evaluate move order!\r\n");
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
                    } else {
                        printf("failed! Defense wins.\r\n");
                        o[vo[k]].valid = 0; //invalidate
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
                    //cut from move or successful convoy
                    if (((ro.order == 1)||((ro.order == 3)&&(ro.type == 0)))&&(ro.tcountry == c)) check = 0;
                    if ((ro.order == 1)&&(ro.country == sc)&&(ro.tcountry == c)&&(a > d)) check = -1;
                }
                if (check == 1) {
                    printf("success! \r\n");
                    o[vo[k]].confirmed = 1;
                } else if (check == 0) {
                    printf("support cutoff by move! invalidating... \r\n");
                    o[vo[k]].valid = 0;
                    o[vo[k]].confirmed = 1;
                } else {
                    printf("support dislodged by supported country move with greater strength! \r\n");
                    o[vo[k]].valid = 0;
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
                    d = g[c].aS; 
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

//evaluates all valid orders and assigns attack and defense strength to countrys
//returns -1 if it finds invalid order, else 0
int evaluate(){
    printf("Evaluating valid orders... ");
    int i;
    //clean(); //clean up all stats before evaluating
    for (i = 0;i < numO;i++) {
        if (o[i].valid == 1) {
            switch (o[i].order) {
                case 0:
                    g[o[i].country].dS++;
                    break;
                case 1:
                    g[o[i].country].aS++;
                    break;
                case 2:
                    g[o[i].country].dS++;
                    if (o[i].tcountry != -1) { //supporting move
                        g[o[i].scountry].aS++;
                    } else {
                        g[o[i].scountry].dS++;
                    }
                    break;
                case 3:
                    g[o[i].country].dS++;
                    if (o[i].type == 0) { //if army convoy order
                        g[o[i].country].aS++;
                    } 
                    break;
                default:
                    return -1;
                    break;
            }
        }
    }
    printf("finished adding attack and defense strengths!\r\n");
    return 0;
}

//find any units without orders and assign them a valid hold order
int insertholds(){
    //Find all units
    order_t torder;
    int i, j;
    int check = 0;
    printf("Finding unordered units and assigning holds to countrys...");
    for (i = 0;i < 48;i++) {
        if (g[i].occupy_type != 2) { //country is occupied  
            //search through orders    
            for (j = 0;j < numO;j++) {
                //mark as found if there is a valid order for unit
                if ((o[j].valid == 1)&&(o[j].country == i)&&(o[j].player == g[i].player)) {
                    check = 1;
                }
            }
            //if not found add hold order for unit
            if (check == 0) {
                printf(" %i,",i);
                torder.player = g[i].player;
                torder.type = g[i].occupy_type;
                torder.country = i;
                torder.order = 0; //TODO add defensive points
                torder.valid = 1;
                o[numO] = torder; //add to existing orders
                numO++; 

            }
            check = 0;
        } 
    }
    printf(" and done!\r\n");
return 0;
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
    numO = 0;
    numinc = 0;
    int k;
    for(k = 0;k < 48;k++) {
        g[k].dS = 0;
        g[k].aS = 0;
    }
    return 0;
}

