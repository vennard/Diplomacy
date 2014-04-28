/*******************************************************************************
 * Main Timing control loop for Diplomacy 
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "order.h"
#include "region.h"
#include "include.h"

#define D_PHASE 1 //will be minutes later, seconds for now
#define R_PHASE 1
#define S_PHASE 1
#define T_ORDERS 5

time_t timestart,gstart,gend;
time_t stimer,etimer;
int gameNum = 0;
int year = 1802;
int gameRunning = 0;
int testseed = 18;
int Rneeded = 0; //variables to eliminate unecessary phases
int Sneeded = 0;
int customorders = 0;

//Reads input from zedboard GPIO TODO just rework entire menu
//Saves game based on binary coding from gpio
int menu() {
    //must save time of game now TODO
    printf("Menu: c - continue with random orders, a - submit custom orders,  0 - start new game, 1 - Exit, more to come later...\r\n");
    int i = -1;
    char input[16];
    while (i == -1) {
        printf("Enter a menu selection:");
        scanf("%s", input);
        if (strcmp(input,"c") == 0) {
            i = 0;    
        } else if (strcmp(input,"0") == 0) {
            printf("selected new game!\r\n");
            i = 0;    
        } else if (strcmp(input,"1") == 0) {
            printf("selected Exit!\r\n");
        } else if (strcmp(input,"a") == 0) {
            customorders = 1;
            printf("Chose to enter custom orders... good luck\r\n");
            i = 0;    
        } else {    
            printf("you typed %s which is clearly just wrong. Wrong!\r\n",input);
        }
    }
    gameNum = 0; //default game 0
    return 0; //0 = start new game
}

//returns status of game GPIO (ie pause button) TODO
int paused() {
    return 0;
}

//checks if the game has been won TODO
int gamewon() {
    return 0;
}

//waits for timer done or btn press 
//TODO add btn press
void timerwait() {
    printf("start timer... ");
    settmr(D_PHASE);
    starttmr();  
    while (checktmr() == 0) sleep(1);
    printf("done!\r\n");
}

int waitloop(int waittime) {
    printf("Starting timer now for %i seconds...\r\n",waittime);
    int t = 0;
    time(&stimer);
    while(t < waittime) {
         if (paused() != 0) menu();
         //sleep(1);
         time(&etimer);
         t = (int) difftime(etimer, stimer);
         //printf("timer at %i seconds.\r\n",t);
    }
    printf("timer complete after %i seconds.\r\n",t);
}

//for now mode - 0 equals random and 1 - user
int getorders(int mode, int seed, int numorders, char file[]) {
    if (mode == 0) {
        return getTestOrders(seed,numorders,file);
    } else {
        return makeOrders();
    }
}

//polling during waits for pause GPIO input
int main(int argc, char *argv[]) {
	//TODO START OF THE TEST ZONE TODO 
	printf("Launching example game\r\n");
	examplegame();
    //runspi((void*)g); //TODO testing loop for SPI
	while(1) {
		sleep(5);
	}
    //TODO END OF TEST ZONE TODO
    printf("Welcome to Diplomacy!\r\n");
    time(&timestart); //get start time
    printf("Current time - %s\r\n",asctime(localtime(&timestart)));
    //Wait for Menu Selection -- load game / new game / clear game / pause menu
    switch (menu()) {
        case 0 : //starting new game
            startnewgame(gameNum);
            time(&gstart); //get time
            printf("Current time - %s\r\n",asctime(localtime(&timestart)));
            gameRunning = 1;
            break;
        default :
            break;
    }
    configure(); //setup cc1101 

    while (gameRunning) {
        printgame(); // debug print of game status
        // ---- Start of Spring ----
        printf("--------- Start of year %i ----------\r\n\r\n",year);
        printf("--------- Spring %i --------- \r\n",year);
        tx_phase_start(0, g);
        waitloop(D_PHASE); //Start of Order & Diplomacy phase
        //timerwait();
        //numO = rx_orders_start(0);
        numO = getorders(customorders,testseed,T_ORDERS,"/tmp/torders");
        testseed+=5;
        arbitor();
        //starting retreat phase
        if (Rneeded) {
            tx_phase_start(1, g);
            Rneeded = 0;
            printf("Starting retreat phase for spring %i.\r\n",year);
            waitloop(R_PHASE);
            //timerwait();
            //numO = rx_orders_start(1);
            numO = getorders(customorders,testseed,T_ORDERS,"/tmp/torders");
            testseed+=5;
            arbitor();
        } else {
            printf("Skipping retreat phase for spring %i.\r\n",year);
        }
        // ---- Start of Fall ----
        printf("\r\n\r\n--------- Fall %i --------- \r\n",year);
        tx_phase_start(0, g);
        waitloop(D_PHASE);
        //timerwait();
            //numO = rx_orders_start(1);
        numO = getorders(customorders,testseed,T_ORDERS,"/tmp/torders");
        testseed+=5;
        arbitor();
        //starting retreat phase
        if (Rneeded) {
            tx_phase_start(1, g);
            Rneeded = 0;
            printf("Starting retreat phase for fall %i.\r\n",year);
            waitloop(R_PHASE);
            //timerwait();
            //numO = rx_orders_start(1);
            numO = getorders(customorders,testseed,T_ORDERS,"/tmp/torders");
            testseed++;
            arbitor();
            //gen orders and call arbitrator
        } else {
            printf("Skipping retreat phase for fall %i.\r\n",year);
        }
        Sneeded = supplycheck();
        if (Sneeded) {
            tx_phase_start(2, g);
            Sneeded = 0;
            printf("Starting supply phase for fall %i.\r\n",year);
            waitloop(S_PHASE);
            //timerwait();
            //numO = rx_orders_start(2);
            numO = getorders(customorders,testseed,T_ORDERS,"/tmp/torders");
            testseed++;
            //arbitor();
            supplyphase();
        } else {
            printf("skipping supply phase for fall %i.\r\n",year);
        }
        printf("\r\nChecking winning conditions... ");
        if (gamewon()) {
            printf("The Game is over!!!!\r\n");
        } else {
            printf("Nope.\r\n");
        }
        printf("\r\n\r\n");
        year++;
        menu();
    }
    return 0;
}


