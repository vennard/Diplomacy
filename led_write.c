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
#define SHIFT_FILE "/sys/kernel/ece453_pl/shift"
#define DISPLAY_FILE "/sys/kernel/ece453_pl/display"
#define DATA_FILE "/sys/kernel/ece453_pl/data"
#define DATASIZE_FILE ""
#define READY_FILE ""
#define TMRMIN_FILE ""
#define TMRSTART_FILE ""
#define TMRMASK_FILE ""
#define TMRUP_FILE ""

static uint8_t leds[39]; //last 6 bits unused 306 / 8 = 38.25

//populate led data with zeros
void initialize(){
    //TODO set datasize!!
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
}

int write_reg(int fd, uint8_t writebuf) {
    uint8_t *val = NULL;
    val = &writebuf;
    if(write(fd, val, sizeof(uint8_t)) == -1) {
        fprintf(stderr, "error writing to file\n");
        return -1;
    } else {
        printf("register written\n");
        return 0;
    }
}

uint8_t read_reg(int fd) {
    uint8_t rd = 0x00;
    if (read(fd, &rd, sizeof(uint8_t)) < 0) {
        fprintf(stderr, "error reading from reg\n");
        return -1;
    } else {
        printf("register read: %i\n",rd);
        return rd;
    }
}

//waits until sees ready in ready_file
void readywait() {
    printf("Waiting to write until system is ready... ");
    uint8_t rdy = 0x00;
    while (rdy != 0x01) {
        int fd = open(READY_FILE, O_WRONLY);
        if (fd < 0) perror("failed to write to ready_file\n");
        rdy = read_reg(fd);
        close(fd);
        rdy = rdy & 0x01;
    }
    printf(" done!\r\n");
}

//pushes data in leds out to the board
void writeout() {
    int fd, i;
    for (i = 0;i < 39;i++) {
        //write 1 byte at a time
        readywait();
        fd = open(DATA_FILE, O_WRONLY);
        if (fd < 0) perror("failed to write to data_file\n");
        write_reg(fd, leds[39 - i]);
        close(fd);

        //shift data over
        readywait();
        fd = open(SHIFT_FILE, O_WRONLY);
        if (fd < 0) perror("failed to write to shift_file\n");
        write_reg(fd, 0x01);
        close(fd);
    }
    printf("Written all led data to registers... pushing to display now\r\n");
    readywait();
    fd = open(DISPLAY_FILE, O_WRONLY);
    if (fd < 0) perror("failed to write to display_file\n");
    write_reg(fd, 0x01);
    close(fd);
}
