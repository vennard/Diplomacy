/*******************************************************************************
 * Main Timing control loop for Diplomacy 
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "order.h"
#include "region.h"
#include "include.h"

#define D_PHASE 3 //will be minutes later, seconds for now
#define R_PHASE 5
#define S_PHASE 5

time_t timestart,gstart,gend;
time_t stimer,etimer;
int gameNum = 0;
int year = 1802;
int gameRunning = 0;
int testseed = 0;
int Rneeded = 0; //variables to eliminate unecessary phases
int Sneeded = 0;

int menu() {
    //Reads input from zedboard GPIO
    //Saves game based on binary coding from gpio
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
    int t = 0;
    time(&stimer);
    while(t < waittime) {
         if (paused) menu();
         //sleep(1);
         time(&etimer);
         t = (int) difftime(etimer, stimer);
         //printf("timer at %i seconds.\r\n",t);
    }
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
        // ---- Start of Spring ----
        printf("--------- Start of year %i ----------\r\n",year);
        printf("It is spring of the year %i. \r\n",year);
        printf("Starting timer now for %i minutes...\r\n",D_PHASE);
        //Wait loop for timer -- poll gpio status meanwhile
        waitloop(D_PHASE);
        printf("Spring %i is over -- calling arbitor.\r\n",year);
	    numO = getTestOrders(testseed,200,"/tmp/torders");
        //arbitor();
        printf("Decisions have been made.\r\n");
        //starting retreat phase
        if (Rneeded) {
            printf("Starting retreat phase for spring %i.\r\n",year);
            waitloop(R_PHASE);
            //gen orders and call arbitrator
        } else {
            printf("Skipping retreat phase for spring %i.\r\n",year);
        }
        // ---- Start of Fall ----
        printf("It is fall of the year %i.\r\n",year);
        waitloop(D_PHASE);
        printf("Orders phase for fall %i is over -- calling arbitor.\r\n",year);
        //starting retreat phase
        if (Rneeded) {
            printf("Starting retreat phase for fall %i.\r\n",year);
            waitloop(R_PHASE);
            //gen orders and call arbitrator
        } else {
            printf("Skipping retreat phase for fall %i.\r\n",year);
        }
        if (Sneeded) {
            printf("Starting supply phase for fall %i.\r\n",year);
            waitloop(S_PHASE);
        } else {
            printf("skipping supply phase for fall %i.\r\n",year);
        }

        printf("Fall %i is over -- calling arbitor.\r\n",year);
        printf("Checking winning conditions -- calling arbitor.\r\n",year);
        if (gamewon()) {
            printf("The Game is over!!!!\r\n",year);
        }
        year++;


   // gameRunning = 0; //TODO remove
        
    }


    return 0;
}


