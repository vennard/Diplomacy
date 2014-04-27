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

static uint8_t leds[39]; //last 6 bits unused 306 / 8 = 38.25
int write_reg(int fd, int writebuf);

//populate led data with zeros
void initialize(){
    int fd = open(DATASIZE_FILE, O_WRONLY);
    if (fd < 0) perror("failed to write to datasize_file\n");
    write_reg(fd, 0x08);
    close(fd);
    int i;
    for(i = 0;i < 39;i++) leds[i] = 0x00;
    printf("turned all leds off.\r\n");
}

//writes val to lednum
//total of 0 -> 307 valid locations
void writeled(int val, int lednum) { 
    uint8_t temp = 0x01;
    int regnum = lednum / 8;
    int shift = lednum % 8;
    temp = temp << shift;
    if (val == 0) {
        temp = ~temp;
        leds[regnum] = leds[regnum] & temp;  
    } else if (val == 1) {
        leds[regnum] = leds[regnum] | temp;  
    } else {
        printf("Invalid selection of led!\r\n");
    }
    printf("wrote to led %i!\r\n",lednum);
}

int write_reg(int fd, int writebuf) {
    int *writeval = NULL;
    writeval = &writebuf;
    if(write(fd, writeval, sizeof(int)) == -1) {
        fprintf(stderr, "error writing to file\n");
        return -1;
    } else {
        printf("Wrote: %i\n",writebuf);
        return 0;
    }
}

uint8_t read_reg(int fd) {
    char buf[1];
    if (read(fd, buf, sizeof(buf)-1) < 0) {
        printf("error reading from register\r\n");
        return -1;
    } else {
        printf(" ----------------------- register read: %i\n",*buf);
        return *buf;
    }
}

//waits until sees ready in ready_file
void readywait() {
    printf("Waiting to write until system is ready... ");
    uint8_t rdy = 0x00;
    while (rdy != 0x01) {
        int fd = open(READY_FILE, O_RDONLY);
        if (fd < 0) perror("failed to read from ready_file\n");
        rdy = read_reg(fd);
        printf("register read: %i\n",rdy);
        rdy = rdy & 0x01;
        close(fd);
    }
    printf(" done!\r\n");
}

//pushes data in leds out to the board
void writeout() {
    int fd, i;
    for (i = 0;i < 39;i++) {
        //write 1 byte at a time
        readywait();
	printf("writing to data file\r\n");
        fd = open(DATA_FILE, O_WRONLY);
        if (fd < 0) perror("failed to write to data_file\n");
        write_reg(fd, leds[39 - i]);
        close(fd);

        //shift data over
        readywait();
	printf("writing to shift file\r\n");
        fd = open(SHIFT_FILE, O_WRONLY);
        if (fd < 0) perror("failed to write to shift_file\n");
        write_reg(fd, 0x01);
        close(fd);
    }
    printf("Written all led data to registers... pushing to display now\r\n");
    readywait();
    printf("writing to display file\r\n");
    fd = open(DISPLAY_FILE, O_WRONLY);
    if (fd < 0) perror("failed to write to display_file\n");
    write_reg(fd, 0x01);
    close(fd);
}

void examplegame(){
	initialize();
	int i, j;
	printf("Writing to leds:\r\n");
	for(i = 0;i < 306;i++) {
		printf("%i: ",i);
		writeled(1, i);	
		writeout();
		printf("\r\n\r\n ",i);
		sleep(1);
	}
	sleep(5);
	printf("Turning off leds:\r\n");
	for(i = 0;i < 306;i++) {
		writeled(0, i);	
		writeout();
		printf("%i ",i);
		sleep(1);
	}
	printf("Finished examplegame code!\r\n");
}
