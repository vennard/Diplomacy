#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "order.h"
#include "region.h"
#include "include.h"

//total leds = 48 * 2 (army/fleet leds) + 3 * 48 (player leds) + 3 * 22 (supplys)
#define TOTAL_LEDS 306

//Registers for led writing from driver - thanks alex!
//TODO place holders, need to find out locations
#define SHIFT_FILE "/sys/kernel/ece453_digidiplo/shift"
#define DISPLAY_FILE "/sys/kernel/ece453_digidiplo/display"
#define DATA_FILE "/sys/kernel/ece453_digidiplo/data"
#define DATASIZE_FILE "/sys/kernel/ece453_digidiplo/datasize"
#define READY_FILE "/sys/kernel/ece453_digidiplo/ready"
#define TMRMIN_FILE "/sys/kernel/ece453_digidiplo/tmrmins"
#define TMRSTART_FILE "/sys/kernel/ece453_digidiplo/tmrstart"
#define TMRMASK_FILE "/sys/kernel/ece453_digidiplo/tmrmaskintr"
#define TMRUP_FILE "/sys/kernel/ece453_digidiplo/tmrtimeup"

uint8_t read_reg(int fd); 
int leds[300];

int write_reg(const char* file, int val) {
    FILE *fp;
    fp = fopen(file, "w");
    if (fp == NULL) perror("failed to open file");
    fprintf(fp,"%x",val);
    //printf("wrote %x to %s!\r\n",val,file);
    fclose(fp);
}


void settmr(int min) {
    printf("Set timer for %i minutes... ",min);
    write_reg(TMRMIN_FILE, min);
}

void starttmr() {
    printf("Started timer... ");
    write_reg(TMRSTART_FILE, 1);
}

//returns 1 if timer is done, 0 otherwise
char checktmr() {
    FILE *fp;
    fp = fopen(TMRUP_FILE,"r");
    if (fp == NULL) perror("failed to open file");
    char in = fgetc(fp);
    fclose(fp);
    return in;
}

//populate led data with zeros
void initialize(){
    int i;
    for(i = 0;i < 294;i++) leds[i] = 0x00;
    printf("turned all leds off.\r\n");
}

//writes val to lednum
//total of 0 -> 307 valid locations
void writeled(int lednum, int val) { 
    leds[lednum] = val;	 
    printf("wrote %i to led %i!\r\n",val,lednum);
}

//waits until sees ready in ready_file
void readywait() {
    FILE *fp;
    printf("Waiting to write until system is ready... ");
    char rdy = 0x00;
    fp = fopen(READY_FILE, "r");
    while (rdy != 0x01) {
        if (fp == NULL) perror("failed to read from ready_file\n");
        rdy = fgetc(fp);
        printf("got: %x\n",rdy);
        rdy = rdy & 0x01;
    }
    fclose(fp);
    printf(" done!\r\n");
}

void shiftone() {
    write_reg(DATASIZE_FILE, 0);
    write_reg(DATA_FILE, 0x80000000);
    write_reg(SHIFT_FILE, 0x1);
}

void shiftzero() {
    write_reg(DATASIZE_FILE, 0);
    write_reg(DATA_FILE, 0x00000000);
    write_reg(SHIFT_FILE, 0x1);
}


//pushes data in leds out to the board
void writeout() {
    int i;
    for(i = 0;i < 285;i++) {
        if (leds[284-i] == 1) shiftone();
	    usleep(5);
        if (leds[284-i] == 0) shiftzero();
    }
    write_reg(DISPLAY_FILE, 0x1);
    printf(" done!\r\n");
}

//type is 0 for army, 1 for fleet, 2 for owner, 3 for supply
//returns 0 on success, 1 on failure
int writeregion(int player, int region, int type) {
    //I wonder if there is math I can do here
    return 0;
}

void clearboard() {
    printf("trying to clear board!");
    initialize();
    writeout();
}

void examplegame(){
	initialize();
    writeled(0,1);
	writeout();
    usleep(100000);
    /*
    clearboard();
	int i;
	printf("Writing to leds:\r\n");
	for(i = 0;i < 306;i++) {
		printf("%i: ",i);
		writeled(1, i);	
		writeout();
		usleep(1000);
	}
	sleep(5);
	printf("\r\nTurning off leds:\r\n");
	for(i = 0;i < 306;i++) {
		writeled(0, i);	
		writeout();
		printf("%i ",i);
		usleep(1000);
	}
*/
	printf("Finished examplegame code!\r\n");
}
