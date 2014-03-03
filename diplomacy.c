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
#define T_ORDERS 10

time_t timestart,gstart,gend;
time_t stimer,etimer;
int gameNum = 0;
int year = 1802;
int gameRunning = 0;
int testseed = 18;
int Rneeded = 0; //variables to eliminate unecessary phases
int Sneeded = 0;

//Reads input from zedboard GPIO TODO just rework entire menu
//Saves game based on binary coding from gpio
int menu() {
    //must save time of game now TODO
    printf("Menu: c - continue,  0 - start new game, 1 - Exit, more to come later...\r\n");
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
        } else {    
            printf("you typed %s which is clearly just wrong. Wrong!\r\n",input);
        }
    }
    gameNum = 0; //default game 0
    return 0; //0 = start new game
}

//returns status of game GPIO (ie pause button)
int paused() {
    return 0;
}

//checks if the game has been won
int gamewon() {
    return 0;
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

//polling during waits for pause GPIO input
int main(int argc, char *argv[]) {
    printf("Welcome to Diplomacy!\r\n");
    time(&timestart); //get start time
    printf("Current time - %s\r\n",asctime(localtime(&timestart)));
    //difftime(time1, time2); //returns difference in seconds
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

    while (gameRunning) {
        // debug print of game status
        printgame(); 
        // ---- Start of Spring ----
        printf("--------- Start of year %i ----------\r\n\r\n",year);
        printf("--------- Spring %i --------- \r\n",year);
        waitloop(D_PHASE); //Start of Order & Diplomacy phase
	    numO = getTestOrders(testseed,T_ORDERS,"/tmp/torders");
        testseed+=5;
        arbitor();
        //starting retreat phase
        if (Rneeded) {
            printf("Starting retreat phase for spring %i.\r\n",year);
            waitloop(R_PHASE);
	        numO = getTestOrders(testseed,T_ORDERS,"/tmp/torders");
            testseed+=5;
            arbitor();
            //gen orders and call arbitrator
        } else {
            printf("Skipping retreat phase for spring %i.\r\n",year);
        }
        // ---- Start of Fall ----
        printf("\r\n\r\n--------- Fall %i --------- \r\n",year);
        waitloop(D_PHASE);
	    numO = getTestOrders(testseed,T_ORDERS,"/tmp/torders");
        testseed+=5;
        arbitor();
        //starting retreat phase
        if (Rneeded) {
            printf("Starting retreat phase for fall %i.\r\n",year);
            waitloop(R_PHASE);
	        numO = getTestOrders(testseed,T_ORDERS,"/tmp/torders");
            testseed++;
            arbitor();
            //gen orders and call arbitrator
        } else {
            printf("Skipping retreat phase for fall %i.\r\n",year);
        }
        if (Sneeded) {
            printf("Starting supply phase for fall %i.\r\n",year);
            waitloop(S_PHASE);
	        numO = getTestOrders(testseed,T_ORDERS,"/tmp/torders");
            testseed++;
            arbitor();
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
        //gameRunning = 0; //TODO remove
    }


    return 0;
}


