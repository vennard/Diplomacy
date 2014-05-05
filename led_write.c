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
void readywait();
void shiftin(int val); 
void initialize();

int write_reg(const char* file, int val) {
    readywait();
    FILE *fp;
    fp = fopen(file, "w");
    if (fp == NULL) perror("failed to open file");
    fprintf(fp,"%x",val);
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

void clearboard() {
    int i;
    write_reg(DATA_FILE, 0);
    usleep(10);
    write_reg(DATASIZE_FILE, 0);
    usleep(10);
    for(i = 0;i < 295;i++){
        write_reg(SHIFT_FILE,1);
        write_reg(DISPLAY_FILE,1);
    }
    printf("should have clear the board!");
    initialize();
}

//pushes data in leds out to the board
void writeout2() {
    int i;
    int c = 0;
    //write_reg(DATASIZE_FILE, 31);
    int out = 0;
    /*
    for (i = 0;i < 32;i++) {
        if(leds[32 - c] == 1) {
            out = out | 0x00000001; //set bit 0 to 1
        } else {
            out = out & 0xfffffffe; //set bit 0 to 0
        }
        out = out << 1; //shift over one bit
    }
    */
    out = 0x80000001;
    printf("Value to write out 0x%x!\r\n",out);
    write_reg(DATA_FILE,out);
    write_reg(SHIFT_FILE,1);
    write_reg(DISPLAY_FILE, 1);
}

void writeout() {
    int i;
    for(i = 0;i < 285;i++) shiftin(leds[284-i]);
    write_reg(DISPLAY_FILE, 0x1);
}


void blink() {
    clearboard();
    int i; 
    for (i = 0;i < 10;i++) {
        writeled(0, 1);
        writeout();
        usleep(100);
    }
}

//populate led data with zeros
void initialize(){
    int i;
    write_reg(DATASIZE_FILE, 0);
    for(i = 0;i < 294;i++) writeled(i,0);
    printf("turned all leds off.\r\n");
}

//writes val to lednum
//total of 0 -> 284 valid index locations
void writeled(int lednum, int val) { 
    if ((lednum > 284)||(lednum < 0)) perror("tried to write to invalid led!\r\n");
    leds[lednum] = val;	 
}

//waits until sees ready in ready_file
void readywait() {
    FILE *fp;
    char rdy = 0x00;
    fp = fopen(READY_FILE, "r");
    while (rdy != 0x01) {
        if (fp == NULL) perror("failed to read from ready_file\n");
        rdy = fgetc(fp);
        rdy = rdy & 0x01;
        //printf("ready %x\r\n",rdy);
    }
    fclose(fp);
}

void shiftin(int val) {
    switch(val) {
        case 0: 
            write_reg(DATA_FILE, 0x00000000);
            write_reg(SHIFT_FILE, 0x1);
            break;
        case 1:
            write_reg(DATA_FILE, 0x80000000);
            write_reg(SHIFT_FILE, 0x1);
            break;
        default:
            perror("tried to shift in invalid val\r\n");
            break;
    }
}



void fancystart() {
    clearboard();
    printf("oh fancy\r\n");
    srand(time(NULL));
    initialize();
    int spread = 20;
    int density = 15;
    int i, j;
    for(i = 0;i < 10;i++) {
        int startloc = rand() % 285;
        for (j = 0;j < density;j++) {
            int plusminus = rand() % 2;
            if (plusminus == 0) plusminus = -1;
            int loc = (plusminus*(rand() % spread)) + startloc;
            if ((loc < 285)&&(loc >= 0)) writeled(loc, 1);
            writeout();
        }
        initialize();
    }
    printf("done!");
    initialize();
    writeout();
}

void logo() {
    printf("You want a logo? fine.\r\n");
    initialize();
    clearboard();
    usleep(10);
    int logo_d1[6] = {262, 260, 184, 240, 243, 257}; 
    int logo_i1[3] = {204, 200, 245};
    int logo_g[12] = {249, 251, 235, 207, 210, 212, 211, 216, 123, 118, 114, 131};
    int logo_i2[4] = {99, 126, 226, 231}; 
    int logo_d2[6] = {183, 185, 188, 165, 176, 153};
    int logo_i3[3] = {142, 144, 170};
    int logo_p[6] = {76, 83, 85, 72, 23, 26};
    int logo_l[3] = {9, 79, 84};
    int logo_o[4] = {44, 42, 35, 37};
    //write logo
    int i;
    for(i = 0;i < 6;i++) writeled(logo_d1[i],1);
    writeout();
    for(i = 0;i < 3;i++) writeled(logo_i1[i],1);
    writeout();
    for(i = 0;i < 12;i++) writeled(logo_g[i],1);
    writeout();
    for(i = 0;i < 4;i++) writeled(logo_i2[i],1);
    writeout();
    for(i = 0;i < 6;i++) writeled(logo_d2[i],1);
    writeout();
    for(i = 0;i < 3;i++) writeled(logo_i3[i],1);
    writeout();
    for(i = 0;i < 6;i++) writeled(logo_p[i],1);
    writeout();
    for(i = 0;i < 3;i++) writeled(logo_l[i],1);
    writeout();
    for(i = 0;i < 4;i++) writeled(logo_o[i],1);
    writeout();
}

void examplegame(){
	initialize();
    sleep(1);
    //writeregion(0);
    sleep(1);
    //writeregion(5);
    sleep(1);
    clearboard();
	int i;
	for(i = 0;i < 306;i++) {
		writeled(i, 1);	
		writeout();
		//usleep(1);
	}
	for(i = 0;i < 306;i++) {
		writeled(i, 0);	
		writeout();
		//usleep(1);
	}
	printf("Finished examplegame code!\r\n");
}


//type is 0 for army, 1 for fleet, 2 for owner, 3 for supply
//returns 0 on success, 1 on failure
//type ignored on countrys with only owner + 1 type
void writeregion(int region) {
    //occupied
    switch (region) {
        case 0: //ode
            if (g[region].occupy_type != 2) {
                writeled(3, 1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(0,1);
                        writeled(1,1);
                        writeled(2,1);
                        break;
                    case 1: //red
                        writeled(0,1);
                        break;
                    case 2: //blue
                        writeled(2,1);
                        break;
                    case 3: //green
                        writeled(1,1);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 1: //vis
            if (g[region].occupy_type != 2) {
                writeled(55, 1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(56,1);
                        writeled(57,1);
                        writeled(58,1);
                        break;
                    case 1: //red
                        writeled(58,1);
                        break;
                    case 2: //blue
                        writeled(56,1);
                        break;
                    case 3: //green
                        writeled(57,1);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 2: //war
            if (g[region].occupy_type != 2) {
                writeled(14,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(8,1);
                        writeled(9,1);
                        writeled(10,1);
                        break;
                    case 1: //red
                        writeled(8,1);
                        break;
                    case 2: //blue
                        writeled(10,1);
                        break;
                    case 3: //green
                        writeled(9,1);
                        break;
                    default:
                        break;
                }
            }
            switch (g[region].supply) {
            case 0: //white
                writeled(11,1);
                writeled(12,1);
                writeled(13,1);
                break;
            case 1: //red
                writeled(11,1);
                break;
            case 2: //blue
                writeled(13,1);
                break;
            case 3: //green
                writeled(12,1);
                break;
            default:
                break;
            }
            break;
        case 3: //bre
            if (g[region].occupy_type != 2) {
                writeled(42,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(15,1);
                        writeled(46,1);
                        writeled(47,1);
                        break;
                    case 1: //red
                        writeled(15,1);
                        break;
                    case 2: //blue
                        writeled(46,1);
                        break;
                    case 3: //green
                        writeled(47,1);
                    break;
                    default:
                    break;
                }

            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(43,1);
                    writeled(44,1);
                    writeled(45,1);
                    break;
                case 1: //red
                    writeled(45,1);
                    break;
                case 2: //blue
                    writeled(43,1);
                    break;
                case 3: //green
                    writeled(44,1);
                    break;
                default:
                    break;
            }
            break;
        case 4: //min
            if (g[region].occupy_type != 2) {
                writeled(35,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(36,1);
                        writeled(37,1);
                        writeled(38,1);
                        break;
                    case 1: //red
                        writeled(38,1);
                        break;
                    case 2: //blue
                        writeled(36,1);
                        break;
                    case 3: //green
                        writeled(37,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(39,1);
                    writeled(40,1);
                    writeled(41,1);
                    break;
                case 1: //red
                    writeled(41,1);
                    break;
                case 2: //blue
                    writeled(39,1);
                    break;
                case 3: //green
                    writeled(40,1);
                    break;
                default:
                    break;
            }
            break;
        case 5: //str 
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(20,1);
                if (g[region].occupy_type == 0) writeled(19,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(32,1);
                        writeled(33,1);
                        writeled(34,1);
                        break;
                    case 1: //red
                        writeled(34,1);
                        break;
                    case 2: //blue
                        writeled(32,1);
                        break;
                    case 3: //green
                        writeled(33,1);
                        break;
                    default:
                        break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(16,1);
                    writeled(17,1);
                    writeled(18,1);
                    break;
                case 1: //red
                    writeled(16,1);
                    break;
                case 2: //blue
                    writeled(18,1);
                    break;
                case 3: //green
                    writeled(17,1);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
        case 6: //dan
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(28,1);
                if (g[region].occupy_type == 0) writeled(27,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(21,1);
                        writeled(22,1);
                        writeled(23,1);
                        break;
                    case 1: //red
                        writeled(21,1);
                        break;
                    case 2: //blue
                        writeled(23,1);
                        break;
                    case 3: //green
                        writeled(22,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(24,1);
                    writeled(25,1);
                    writeled(26,1);
                    break;
                case 1: //red
                    writeled(24,1);
                    break;
                case 2: //blue
                    writeled(26,1);
                    break;
                case 3: //green
                    writeled(25,1);
                    break;
                default:
                    break;
            }
            break;
        case 7: //wkr
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(63,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(29,1);
                        writeled(30,1);
                        writeled(31,1);
                        break;
                    case 1: //red
                        writeled(29,1);
                        break;
                    case 2: //blue
                        writeled(31,1);
                        break;
                    case 3: //green
                        writeled(30,1);
                    break;
                    default:
                    break;
                }
            }
        case 8: //nar
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(59,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(60,1);
                        writeled(61,1);
                        writeled(62,1);
                        break;
                    case 1: //red
                        writeled(62,1);
                        break;
                    case 2: //blue
                        writeled(60,1);
                        break;
                    case 3: //green
                        writeled(61,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 9: //wes
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(7,1);
                if (g[region].occupy_type == 1) writeled(54,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(4,1);
                        writeled(5,1);
                        writeled(6,1);
                        break;
                    case 1: //red
                        writeled(4,1);
                        break;
                    case 2: //blue
                        writeled(6,1);
                        break;
                    case 3: //green
                        writeled(5,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 10: //kat
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(50,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(51,1);
                        writeled(52,1);
                        writeled(53,1);
                        break;
                    case 1: //red
                        writeled(53,1);
                        break;
                    case 2: //blue
                        writeled(51,1);
                        break;
                    case 3: //green
                        writeled(52,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 11: //cop TODO check these values
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(64,1);
                if (g[region].occupy_type == 0) writeled(66,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(48,1);
                        writeled(49,1);
                        writeled(67,1);
                        break;
                    case 1: //red
                        writeled(49,1);
                        break;
                    case 2: //blue
                        writeled(67,1);
                        break;
                    case 3: //green
                        writeled(48,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(65,1);
                    writeled(68,1);
                    writeled(69,1);
                    break;
                case 1: //red
                    writeled(65,1);
                    break;
                case 2: //blue
                    writeled(69,1);
                    break;
                case 3: //green
                    writeled(68,1);
                    break;
                default:
                    break;
            }
            break;
        case 12: //sba
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(73,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(70,1);
                        writeled(71,1);
                        writeled(72,1);
                        break;
                    case 1: //red
                        writeled(70,1);
                        break;
                    case 2: //blue
                        writeled(72,1);
                        break;
                    case 3: //green
                        writeled(71,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 13: //mba
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(77,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(70,1);
                        writeled(71,1);
                        writeled(72,1);
                        break;
                    case 1: //red
                        writeled(70,1);
                        break;
                    case 2: //blue
                        writeled(72,1);
                        break;
                    case 3: //green
                        writeled(71,1);
                    break;
                    default:
                    break;
                }
            }
            break;
         case 14: //kon
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(85,1);
                if (g[region].occupy_type == 0) writeled(84,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(78,1);
                        writeled(79,1);
                        writeled(80,1);
                        break;
                    case 1: //red
                        writeled(78,1);
                        break;
                    case 2: //blue
                        writeled(80,1);
                        break;
                    case 3: //green
                        writeled(79,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(81,1);
                    writeled(82,1);
                    writeled(83,1);
                    break;
                case 1: //red
                    writeled(81,1);
                    break;
                case 2: //blue
                    writeled(83,1);
                    break;
                case 3: //green
                    writeled(82,1);
                    break;
                default:
                    break;
            }
            break;
        case 15: //ron
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(93,1);
                if (g[region].occupy_type == 0) writeled(92,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(86,1);
                        writeled(87,1);
                        writeled(88,1);
                        break;
                    case 1: //red
                        writeled(86,1);
                        break;
                    case 2: //blue
                        writeled(88,1);
                        break;
                    case 3: //green
                        writeled(87,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(89,1);
                    writeled(90,1);
                    writeled(91,1);
                    break;
                case 1: //red
                    writeled(89,1);
                    break;
                case 2: //blue
                    writeled(91,1);
                    break;
                case 3: //green
                    writeled(90,1);
                    break;
                default:
                    break;
            }
            break;
        case 16: //hab
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(110,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(111,1);
                        writeled(94,1);
                        writeled(95,1);
                        break;
                    case 1: //red
                        writeled(94,1);
                        break;
                    case 2: //blue
                        writeled(111,1);
                        break;
                    case 3: //green
                        writeled(95,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 17: //rig
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(102,1);
                if (g[region].occupy_type == 0) writeled(103,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(104,1);
                        writeled(105,1);
                        writeled(106,1);
                        break;
                    case 1: //red
                        writeled(106,1);
                        break;
                    case 2: //blue
                        writeled(104,1);
                        break;
                    case 3: //green
                        writeled(105,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(107,1);
                    writeled(108,1);
                    writeled(109,1);
                    break;
                case 1: //red
                    writeled(109,1);
                    break;
                case 2: //blue
                    writeled(107,1);
                    break;
                case 3: //green
                    writeled(108,1);
                    break;
                default:
                    break;
            }
            break;
        case 18: //stp
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(126,1);
                if (g[region].occupy_type == 0) writeled(127,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(96,1);
                        writeled(97,1);
                        writeled(98,1);
                        break;
                    case 1: //red
                        writeled(98,1);
                        break;
                    case 2: //blue
                        writeled(96,1);
                        break;
                    case 3: //green
                        writeled(97,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(99,1);
                    writeled(100,1);
                    writeled(101,1);
                    break;
                case 1: //red
                    writeled(101,1);
                    break;
                case 2: //blue
                    writeled(99,1);
                    break;
                case 3: //green
                    writeled(100,1);
                    break;
                default:
                    break;
            }
            break;
        case 19: //tal
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(118,1);
                if (g[region].occupy_type == 0) writeled(119,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(123,1);
                        writeled(124,1);
                        writeled(125,1);
                        break;
                    case 1: //red
                        writeled(125,1);
                        break;
                    case 2: //blue
                        writeled(123,1);
                        break;
                    case 3: //green
                        writeled(124,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(120,1);
                    writeled(121,1);
                    writeled(122,1);
                    break;
                case 1: //red
                    writeled(122,1);
                    break;
                case 2: //blue
                    writeled(120,1);
                    break;
                case 3: //green
                    writeled(121,1);
                    break;
                default:
                    break;
            }
            break;
        case 20: //gor
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(114,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(115,1);
                        writeled(116,1);
                        writeled(117,1);
                        break;
                    case 1: //red
                        writeled(117,1);
                        break;
                    case 2: //blue
                        writeled(115,1);
                        break;
                    case 3: //green
                        writeled(116,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 21: //saa
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(133,1);
                if (g[region].occupy_type == 0) writeled(132,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(112,1);
                        writeled(113,1);
                        writeled(128,1);
                        break;
                    case 1: //red
                        writeled(113,1);
                        break;
                    case 2: //blue
                        writeled(128,1);
                        break;
                    case 3: //green
                        writeled(112,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(129,1);
                    writeled(130,1);
                    writeled(131,1);
                    break;
                case 1: //red
                    writeled(129,1);
                    break;
                case 2: //blue
                    writeled(131,1);
                    break;
                case 3: //green
                    writeled(130,1);
                    break;
                default:
                    break;
            }
            break;
        case 22: //nba
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(137,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(134,1);
                        writeled(135,1);
                        writeled(136,1);
                        break;
                    case 1: //red
                        writeled(134,1);
                        break;
                    case 2: //blue
                        writeled(136,1);
                        break;
                    case 3: //green
                        writeled(135,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 23: //sli
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(145,1);
                if (g[region].occupy_type == 0) writeled(144,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(138,1);
                        writeled(139,1);
                        writeled(140,1);
                        break;
                    case 1: //red
                        writeled(138,1);
                        break;
                    case 2: //blue
                        writeled(140,1);
                        break;
                    case 3: //green
                        writeled(139,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(141,1);
                    writeled(142,1);
                    writeled(143,1);
                    break;
                case 1: //red
                    writeled(141,1);
                    break;
                case 2: //blue
                    writeled(143,1);
                    break;
                case 3: //green
                    writeled(142,1);
                    break;
                default:
                    break;
            }
            break;
        case 24: //gos
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(168,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(169,1);
                        writeled(170,1);
                        writeled(171,1);
                        break;
                    case 1: //red
                        writeled(171,1);
                        break;
                    case 2: //blue
                        writeled(169,1);
                        break;
                    case 3: //green
                        writeled(170,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 25: //bol
        if (g[region].occupy_type != 2) {
                int coast = rand() % 2;
                coast = coast + 154;
                if (g[region].occupy_type == 1) writeled(coast,1); //TODO add coast support here
                if (g[region].occupy_type == 0) writeled(153,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(150,1);
                        writeled(151,1);
                        writeled(152,1);
                        break;
                    case 1: //red
                        writeled(150,1);
                        break;
                    case 2: //blue
                        writeled(152,1);
                        break;
                    case 3: //green
                        writeled(151,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 26: //mal
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(172,1);
                if (g[region].occupy_type == 0) writeled(173,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(156,1);
                        writeled(157,1);
                        writeled(158,1);
                        break;
                    case 1: //red
                        writeled(156,1);
                        break;
                    case 2: //blue
                        writeled(158,1);
                        break;
                    case 3: //green
                        writeled(157,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(174,1);
                    writeled(175,1);
                    writeled(159,1);
                    break;
                case 1: //red
                    writeled(159,1);
                    break;
                case 2: //blue
                    writeled(174,1);
                    break;
                case 3: //green
                    writeled(175,1);
                    break;
                default:
                    break;
            }
            break;
        case 27: //ska
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(149,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(146,1);
                        writeled(147,1);
                        writeled(148,1);
                        break;
                    case 1: //red
                        writeled(146,1);
                        break;
                    case 2: //blue
                        writeled(148,1);
                        break;
                    case 3: //green
                        writeled(147,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 28: //got
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(160,1);
                if (g[region].occupy_type == 0) writeled(161,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(165,1);
                        writeled(166,1);
                        writeled(167,1);
                        break;
                    case 1: //red
                        writeled(167,1);
                        break;
                    case 2: //blue
                        writeled(165,1);
                        break;
                    case 3: //green
                        writeled(163,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(162,1);
                    writeled(163,1);
                    writeled(164,1);
                    break;
                case 1: //red
                    writeled(164,1);
                    break;
                case 2: //blue
                    writeled(162,1);
                    break;
                case 3: //green
                    writeled(163,1);
                    break;
                default:
                    break;
            }
         break;
         case 29: //van
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(188,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(189,1);
                        writeled(190,1);
                        writeled(191,1);
                        break;
                    case 1: //red
                        writeled(191,1);
                        break;
                    case 2: //blue
                        writeled(189,1);
                        break;
                    case 3: //green
                        writeled(190,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 30: //sil
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(183,1); 
                if (g[region].occupy_type == 0) writeled(184,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(185,1);
                        writeled(186,1);
                        writeled(187,1);
                        break;
                    case 1: //red
                        writeled(187,1);
                        break;
                    case 2: //blue
                        writeled(185,1);
                        break;
                    case 3: //green
                        writeled(186,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 31: //sto
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(192,1);
                if (g[region].occupy_type == 0) writeled(176,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(178,1);
                        writeled(179,1);
                        writeled(177,1);
                        break;
                    case 1: //red
                        writeled(179,1);
                        break;
                    case 2: //blue
                        writeled(177,1);
                        break;
                    case 3: //green
                        writeled(178,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(180,1);
                    writeled(181,1);
                    writeled(182,1);
                    break;
                case 1: //red
                    writeled(182,1);
                    break;
                case 2: //blue
                    writeled(180,1);
                    break;
                case 3: //green
                    writeled(181,1);
                    break;
                default:
                    break;
            }
         break;
         case 32: //ala
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(200,1);
                if (g[region].occupy_type == 0) writeled(199,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(193,1);
                        writeled(194,1);
                        writeled(195,1);
                        break;
                    case 1: //red
                        writeled(193,1);
                        break;
                    case 2: //blue
                        writeled(195,1);
                        break;
                    case 3: //green
                        writeled(194,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(196,1);
                    writeled(197,1);
                    writeled(198,1);
                    break;
                case 1: //red
                    writeled(196,1);
                    break;
                case 2: //blue
                    writeled(198,1);
                    break;
                case 3: //green
                    writeled(197,1);
                    break;
                default:
                    break;
            }
         break;
        case 33: //sgo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(204,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(201,1);
                        writeled(202,1);
                        writeled(203,1);
                        break;
                    case 1: //red
                        writeled(201,1);
                        break;
                    case 2: //blue
                        writeled(203,1);
                        break;
                    case 3: //green
                        writeled(202,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 34: //hel
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(212,1);
                if (g[region].occupy_type == 0) writeled(211,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(208,1);
                        writeled(209,1);
                        writeled(210,1);
                        break;
                    case 1: //red
                        writeled(208,1);
                        break;
                    case 2: //blue
                        writeled(210,1);
                        break;
                    case 3: //green
                        writeled(211,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(205,1);
                    writeled(206,1);
                    writeled(207,1);
                    break;
                case 1: //red
                    writeled(205,1);
                    break;
                case 2: //blue
                    writeled(207,1);
                    break;
                case 3: //green
                    writeled(206,1);
                    break;
                default:
                    break;
            }
        break;
        case 35: //gof
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(216,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(213,1);
                        writeled(214,1);
                        writeled(215,1);
                        break;
                    case 1: //red
                        writeled(213,1);
                        break;
                    case 2: //blue
                        writeled(215,1);
                        break;
                    case 3: //green
                        writeled(214,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 36: //lla
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(220,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(217,1);
                        writeled(218,1);
                        writeled(219,1);
                        break;
                    case 1: //red
                        writeled(217,1);
                        break;
                    case 2: //blue
                        writeled(219,1);
                        break;
                    case 3: //green
                        writeled(218,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 37: //kuy
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(238,1); 
                if (g[region].occupy_type == 0) writeled(239,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(221,1);
                        writeled(222,1);
                        writeled(223,1);
                        break;
                    case 1: //red
                        writeled(221,1);
                        break;
                    case 2: //blue
                        writeled(223,1);
                        break;
                    case 3: //green
                        writeled(222,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 38: //kuo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(230,1);
                if (g[region].occupy_type == 0) writeled(231,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(235,1);
                        writeled(236,1);
                        writeled(237,1);
                        break;
                    case 1: //red
                        writeled(237,1);
                        break;
                    case 2: //blue
                        writeled(235,1);
                        break;
                    case 3: //green
                        writeled(236,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(232,1);
                    writeled(233,1);
                    writeled(234,1);
                    break;
                case 1: //red
                    writeled(234,1);
                    break;
                case 2: //blue
                    writeled(232,1);
                    break;
                case 3: //green
                    writeled(233,1);
                    break;
                default:
                    break;
            }
        break;
        case 39: //sai
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(225,1); 
                if (g[region].occupy_type == 0) writeled(226,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(227,1);
                        writeled(228,1);
                        writeled(229,1);
                        break;
                    case 1: //red
                        writeled(229,1);
                        break;
                    case 2: //blue
                        writeled(227,1);
                        break;
                    case 3: //green
                        writeled(228,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 40: //vas
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(249,1);
                if (g[region].occupy_type == 0) writeled(250,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(252,1);
                        writeled(253,1);
                        writeled(251,1);
                        break;
                    case 1: //red
                        writeled(253,1);
                        break;
                    case 2: //blue
                        writeled(251,1);
                        break;
                    case 3: //green
                        writeled(252,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(224,1);
                    writeled(254,1);
                    writeled(255,1);
                    break;
                case 1: //red
                    writeled(224,1);
                    break;
                case 2: //blue
                    writeled(254,1);
                    break;
                case 3: //green
                    writeled(255,1);
                    break;
                default:
                    break;
            }
        break;
        case 41: //ngo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(245,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(246,1);
                        writeled(247,1);
                        writeled(248,1);
                        break;
                    case 1: //red
                        writeled(248,1);
                        break;
                    case 2: //blue
                        writeled(246,1);
                        break;
                    case 3: //green
                        writeled(247,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 42: //sun
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(256,1);
                if (g[region].occupy_type == 0) writeled(257,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(258,1);
                        writeled(240,1);
                        writeled(241,1);
                        break;
                    case 1: //red
                        writeled(241,1);
                        break;
                    case 2: //blue
                        writeled(258,1);
                        break;
                    case 3: //green
                        writeled(240,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(242,1);
                    writeled(243,1);
                    writeled(244,1);
                    break;
                case 1: //red
                    writeled(244,1);
                    break;
                case 2: //blue
                    writeled(242,1);
                    break;
                case 3: //green
                    writeled(243,1);
                    break;
                default:
                    break;
            }
        break;
        case 43: //stj
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(262,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(260,1);
                        writeled(261,1);
                        writeled(259,1);
                        break;
                    case 1: //red
                        writeled(259,1);
                        break;
                    case 2: //blue
                        writeled(261,1);
                        break;
                    case 3: //green
                        writeled(260,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 44: //keb
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(267,1); 
                if (g[region].occupy_type == 0) writeled(266,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(263,1);
                        writeled(264,1);
                        writeled(265,1);
                        break;
                    case 1: //red
                        writeled(263,1);
                        break;
                    case 2: //blue
                        writeled(265,1);
                        break;
                    case 3: //green
                        writeled(264,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 45: //tor
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(275,1);
                if (g[region].occupy_type == 0) writeled(274,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(268,1);
                        writeled(269,1);
                        writeled(270,1);
                        break;
                    case 1: //red
                        writeled(268,1);
                        break;
                    case 2: //blue
                        writeled(270,1);
                        break;
                    case 3: //green
                        writeled(269,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(271,1);
                    writeled(272,1);
                    writeled(273,1);
                    break;
                case 1: //red
                    writeled(271,1);
                    break;
                case 2: //blue
                    writeled(273,1);
                    break;
                case 3: //green
                    writeled(272,1);
                    break;
                default:
                    break;
            }
        break;
        case 46: //oul
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(280,1); 
                if (g[region].occupy_type == 0) writeled(279,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(276,1);
                        writeled(278,1);
                        writeled(277,1);
                        break;
                    case 1: //red
                        writeled(276,1);
                        break;
                    case 2: //blue
                        writeled(278,1);
                        break;
                    case 3: //green
                        writeled(277,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 47: //ima
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(284,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(281,1);
                        writeled(282,1);
                        writeled(283,1);
                        break;
                    case 1: //red
                        writeled(281,1);
                        break;
                    case 2: //blue
                        writeled(283,1);
                        break;
                    case 3: //green
                        writeled(282,1);
                    break;
                    default:
                    break;
                }
            }
        break;
    writeout();
    }
}

//type is 0 for army, 1 for fleet, 2 for owner, 3 for supply
//returns 0 on success, 1 on failure
//type ignored on countrys with only owner + 1 type
void writeregion2(int region) {
    //occupied
    switch (region) {
        case 0: //ode
            if (g[region].occupy_type != 2) {
                writeled(3, 1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(0,1);
                        writeled(1,1);
                        writeled(2,1);
                        break;
                    case 1: //dont care lol
                        writeled(0,1); //red
                        writeled(1,1); //green
                        break;
                    case 2: //purple
                        writeled(2,1); //blue
                        writeled(0,1); //red 
                        break;
                    case 3: //turqouise 
                        writeled(1,1); //green
                        writeled(2,1); //blue
                        break;
                    default:
                        break;
                }
            }
            break;
        case 1: //vis
            if (g[region].occupy_type != 2) {
                writeled(55, 1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(56,1);
                        writeled(57,1);
                        writeled(58,1);
                        break;
                    case 1: //red
                        writeled(58,1);
                        writeled(57,1);
                        break;
                    case 2: //blue
                        writeled(56,1);
                        writeled(58,1);
                        break;
                    case 3: //green
                        writeled(57,1);
                        writeled(56,1);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 2: //war
            if (g[region].occupy_type != 2) {
                writeled(14,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(8,1);
                        writeled(9,1);
                        writeled(10,1);
                        break;
                    case 1: //red
                        writeled(8,1);
                        writeled(9,1);
                        break;
                    case 2: //blue
                        writeled(10,1);
                        writeled(8,1);
                        break;
                    case 3: //green
                        writeled(9,1);
                        writeled(10,1);
                        break;
                    default:
                        break;
                }
            }
            switch (g[region].supply) {
            case 0: //white
                writeled(11,1);
                writeled(12,1);
                writeled(13,1);
                break;
            case 1: //red
                writeled(11,1);
                writeled(12,1);
                break;
            case 2: //blue
                writeled(13,1);
                writeled(11,1);
                break;
            case 3: //green
                writeled(12,1);
                writeled(13,1);
                break;
            default:
                break;
            }
            break;
        case 3: //bre
            if (g[region].occupy_type != 2) {
                writeled(42,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(15,1);
                        writeled(46,1);
                        writeled(47,1);
                        break;
                    case 1: //red
                        writeled(15,1);
                        writeled(47,1);
                        break;
                    case 2: //blue
                        writeled(46,1);
                        writeled(15,1);
                        break;
                    case 3: //green
                        writeled(47,1);
                        writeled(46,1);
                    break;
                    default:
                    break;
                }

            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(43,1);
                    writeled(44,1);
                    writeled(45,1);
                    break;
                case 1: //red
                    writeled(45,1);
                    writeled(44,1);
                    break;
                case 2: //blue
                    writeled(43,1);
                    writeled(45,1);
                    break;
                case 3: //green
                    writeled(44,1);
                    writeled(43,1);
                    break;
                default:
                    break;
            }
            break;
        case 4: //min
            if (g[region].occupy_type != 2) {
                writeled(35,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(36,1);
                        writeled(37,1);
                        writeled(38,1);
                        break;
                    case 1: //red
                        writeled(38,1);
                        writeled(37,1);
                        break;
                    case 2: //blue
                        writeled(36,1);
                        writeled(38,1);
                        break;
                    case 3: //green
                        writeled(37,1);
                        writeled(39,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(39,1);
                    writeled(40,1);
                    writeled(41,1);
                    break;
                case 1: //red
                    writeled(41,1);
                    writeled(40,1);
                    break;
                case 2: //blue
                    writeled(39,1);
                    writeled(41,1);
                    break;
                case 3: //green
                    writeled(40,1);
                    writeled(39,1);
                    break;
                default:
                    break;
            }
            break;
        case 5: //str 
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(20,1);
                if (g[region].occupy_type == 0) writeled(19,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(32,1);
                        writeled(33,1);
                        writeled(34,1);
                        break;
                    case 1: //red
                        writeled(34,1);
                        writeled(33,1);
                        break;
                    case 2: //blue
                        writeled(32,1);
                        writeled(34,1);
                        break;
                    case 3: //green
                        writeled(33,1);
                        writeled(32,1);
                        break;
                    default:
                        break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(16,1);
                    writeled(17,1);
                    writeled(18,1);
                    break;
                case 1: //red
                    writeled(16,1);
                    writeled(17,1);
                    break;
                case 2: //blue
                    writeled(18,1);
                    writeled(16,1);
                    break;
                case 3: //green
                    writeled(17,1);
                    writeled(18,1);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
        case 6: //dan
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(28,1);
                if (g[region].occupy_type == 0) writeled(27,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(21,1);
                        writeled(22,1);
                        writeled(23,1);
                        break;
                    case 1: //red
                        writeled(21,1);
                        writeled(22,1);
                        break;
                    case 2: //blue
                        writeled(23,1);
                        writeled(21,1);
                        break;
                    case 3: //green
                        writeled(22,1);
                        writeled(23,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(24,1);
                    writeled(25,1);
                    writeled(26,1);
                    break;
                case 1: //red
                    writeled(24,1);
                    writeled(25,1);
                    break;
                case 2: //blue
                    writeled(26,1);
                    writeled(24,1);
                    break;
                case 3: //green
                    writeled(25,1);
                    writeled(26,1);
                    break;
                default:
                    break;
            }
            break;
        case 7: //wkr
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(63,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(29,1);
                        writeled(30,1);
                        writeled(31,1);
                        break;
                    case 1: //red
                        writeled(29,1);
                        writeled(30,1);
                        break;
                    case 2: //blue
                        writeled(31,1);
                        writeled(29,1);
                        break;
                    case 3: //green
                        writeled(30,1);
                        writeled(31,1);
                    break;
                    default:
                    break;
                }
            }
        case 8: //nar
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(59,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(60,1);
                        writeled(61,1);
                        writeled(62,1);
                        break;
                    case 1: //red
                        writeled(62,1);
                        writeled(61,1);
                        break;
                    case 2: //blue
                        writeled(60,1);
                        writeled(62,1);
                        break;
                    case 3: //green
                        writeled(61,1);
                        writeled(60,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 9: //wes
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(7,1);
                if (g[region].occupy_type == 1) writeled(54,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(4,1);
                        writeled(5,1);
                        writeled(6,1);
                        break;
                    case 1: //red
                        writeled(4,1);
                        writeled(5,1);
                        break;
                    case 2: //blue
                        writeled(6,1);
                        writeled(4,1);
                        break;
                    case 3: //green
                        writeled(5,1);
                        writeled(6,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 10: //kat
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(50,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(51,1);
                        writeled(52,1);
                        writeled(53,1);
                        break;
                    case 1: //red
                        writeled(53,1);
                        writeled(52,1);
                        break;
                    case 2: //blue
                        writeled(51,1);
                        writeled(53,1);
                        break;
                    case 3: //green
                        writeled(52,1);
                        writeled(51,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 11: //cop TODO check these values
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(64,1);
                if (g[region].occupy_type == 0) writeled(66,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(48,1);
                        writeled(49,1);
                        writeled(67,1);
                        break;
                    case 1: //red
                        writeled(49,1);
                        writeled(48,1);
                        break;
                    case 2: //blue
                        writeled(67,1);
                        writeled(49,1);
                        break;
                    case 3: //green
                        writeled(48,1);
                        writeled(67,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(65,1);
                    writeled(68,1);
                    writeled(69,1);
                    break;
                case 1: //red
                    writeled(65,1);
                    writeled(68,1);
                    break;
                case 2: //blue
                    writeled(69,1);
                    writeled(65,1);
                    break;
                case 3: //green
                    writeled(68,1);
                    writeled(69,1);
                    break;
                default:
                    break;
            }
            break;
        case 12: //sba
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(73,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(70,1);
                        writeled(71,1);
                        writeled(72,1);
                        break;
                    case 1: //red
                        writeled(70,1);
                        writeled(71,1);
                        break;
                    case 2: //blue
                        writeled(72,1);
                        writeled(70,1);
                        break;
                    case 3: //green
                        writeled(71,1);
                        writeled(72,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 13: //mba
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(77,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(70,1);
                        writeled(71,1);
                        writeled(72,1);
                        break;
                    case 1: //red
                        writeled(70,1);
                        writeled(71,1);
                        break;
                    case 2: //blue
                        writeled(72,1);
                        writeled(70,1);
                        break;
                    case 3: //green
                        writeled(71,1);
                        writeled(72,1);
                    break;
                    default:
                    break;
                }
            }
            break;
         case 14: //kon
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(85,1);
                if (g[region].occupy_type == 0) writeled(84,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(78,1);
                        writeled(79,1);
                        writeled(80,1);
                        break;
                    case 1: //red
                        writeled(78,1);
                        writeled(79,1);
                        break;
                    case 2: //blue
                        writeled(80,1);
                        writeled(78,1);
                        break;
                    case 3: //green
                        writeled(79,1);
                        writeled(80,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(81,1);
                    writeled(82,1);
                    writeled(83,1);
                    break;
                case 1: //red
                    writeled(81,1);
                    writeled(82,1);
                    break;
                case 2: //blue
                    writeled(83,1);
                    writeled(81,1);
                    break;
                case 3: //green
                    writeled(82,1);
                    writeled(83,1);
                    break;
                default:
                    break;
            }
            break;
        case 15: //ron
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(93,1);
                if (g[region].occupy_type == 0) writeled(92,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(86,1);
                        writeled(87,1);
                        writeled(88,1);
                        break;
                    case 1: //red
                        writeled(86,1);
                        writeled(87,1);
                        break;
                    case 2: //blue
                        writeled(88,1);
                        writeled(86,1);
                        break;
                    case 3: //green
                        writeled(87,1);
                        writeled(88,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(89,1);
                    writeled(90,1);
                    writeled(91,1);
                    break;
                case 1: //red
                    writeled(89,1);
                    writeled(90,1);
                    break;
                case 2: //blue
                    writeled(91,1);
                    writeled(89,1);
                    break;
                case 3: //green
                    writeled(90,1);
                    writeled(91,1);
                    break;
                default:
                    break;
            }
            break;
        case 16: //hab
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(110,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(111,1);
                        writeled(94,1);
                        writeled(95,1);
                        break;
                    case 1: //red
                        writeled(94,1);
                        writeled(95,1);
                        break;
                    case 2: //blue
                        writeled(111,1);
                        writeled(94,1);
                        break;
                    case 3: //green
                        writeled(95,1);
                        writeled(111,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 17: //rig
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(102,1);
                if (g[region].occupy_type == 0) writeled(103,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(104,1);
                        writeled(105,1);
                        writeled(106,1);
                        break;
                    case 1: //red
                        writeled(106,1);
                        writeled(105,1);
                        break;
                    case 2: //blue
                        writeled(104,1);
                        writeled(106,1);
                        break;
                    case 3: //green
                        writeled(105,1);
                        writeled(104,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(107,1);
                    writeled(108,1);
                    writeled(109,1);
                    break;
                case 1: //red
                    writeled(109,1);
                    writeled(108,1);
                    break;
                case 2: //blue
                    writeled(107,1);
                    writeled(109,1);
                    break;
                case 3: //green
                    writeled(108,1);
                    writeled(107,1);
                    break;
                default:
                    break;
            }
            break;
        case 18: //stp
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(126,1);
                if (g[region].occupy_type == 0) writeled(127,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(96,1);
                        writeled(97,1);
                        writeled(98,1);
                        break;
                    case 1: //red
                        writeled(98,1);
                        writeled(97,1);
                        break;
                    case 2: //blue
                        writeled(96,1);
                        writeled(98,1);
                        break;
                    case 3: //green
                        writeled(97,1);
                        writeled(96,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(99,1);
                    writeled(100,1);
                    writeled(101,1);
                    break;
                case 1: //red
                    writeled(101,1);
                    writeled(100,1);
                    break;
                case 2: //blue
                    writeled(99,1);
                    writeled(101,1);
                    break;
                case 3: //green
                    writeled(100,1);
                    writeled(99,1);
                    break;
                default:
                    break;
            }
            break;
        case 19: //tal
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(118,1);
                if (g[region].occupy_type == 0) writeled(119,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(123,1);
                        writeled(124,1);
                        writeled(125,1);
                        break;
                    case 1: //red
                        writeled(125,1);
                        writeled(124,1);
                        break;
                    case 2: //blue
                        writeled(123,1);
                        writeled(125,1);
                        break;
                    case 3: //green
                        writeled(124,1);
                        writeled(123,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(120,1);
                    writeled(121,1);
                    writeled(122,1);
                    break;
                case 1: //red
                    writeled(122,1);
                    writeled(121,1);
                    break;
                case 2: //blue
                    writeled(120,1);
                    writeled(122,1);
                    break;
                case 3: //green
                    writeled(121,1);
                    writeled(120,1);
                    break;
                default:
                    break;
            }
            break;
        case 20: //gor
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(114,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(115,1);
                        writeled(116,1);
                        writeled(117,1);
                        break;
                    case 1: //red
                        writeled(117,1);
                        writeled(116,1);
                        break;
                    case 2: //blue
                        writeled(115,1);
                        writeled(117,1);
                        break;
                    case 3: //green
                        writeled(116,1);
                        writeled(115,1);
                    break;
                    default:
                    break;
                }
            }
            break;
        case 21: //saa
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(133,1);
                if (g[region].occupy_type == 0) writeled(132,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(112,1);
                        writeled(113,1);
                        writeled(128,1);
                        break;
                    case 1: //red
                        writeled(113,1);
                        writeled(112,1);
                        break;
                    case 2: //blue
                        writeled(128,1);
                        writeled(113,1);
                        break;
                    case 3: //green
                        writeled(112,1);
                        writeled(128,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(129,1);
                    writeled(130,1);
                    writeled(131,1);
                    break;
                case 1: //red
                    writeled(129,1);
                    writeled(130,1);
                    break;
                case 2: //blue
                    writeled(131,1);
                    writeled(129,1);
                    break;
                case 3: //green
                    writeled(130,1);
                    writeled(131,1);
                    break;
                default:
                    break;
            }
            break;
        case 22: //nba
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(137,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(134,1);
                        writeled(135,1);
                        writeled(136,1);
                        break;
                    case 1: //red
                        writeled(134,1);
                        writeled(135,1);
                        break;
                    case 2: //blue
                        writeled(136,1);
                        writeled(134,1);
                        break;
                    case 3: //green
                        writeled(135,1);
                        writeled(136,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 23: //sli
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(145,1);
                if (g[region].occupy_type == 0) writeled(144,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(138,1);
                        writeled(139,1);
                        writeled(140,1);
                        break;
                    case 1: //red
                        writeled(138,1);
                        writeled(139,1);
                        break;
                    case 2: //blue
                        writeled(140,1);
                        writeled(138,1);
                        break;
                    case 3: //green
                        writeled(139,1);
                        writeled(140,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(141,1);
                    writeled(142,1);
                    writeled(143,1);
                    break;
                case 1: //red
                    writeled(141,1);
                    writeled(142,1);
                    break;
                case 2: //blue
                    writeled(143,1);
                    writeled(141,1);
                    break;
                case 3: //green
                    writeled(142,1);
                    writeled(143,1);
                    break;
                default:
                    break;
            }
            break;
        case 24: //gos
        if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(168,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(169,1);
                        writeled(170,1);
                        writeled(171,1);
                        break;
                    case 1: //red
                        writeled(171,1);
                        writeled(170,1);
                        break;
                    case 2: //blue
                        writeled(169,1);
                        writeled(171,1);
                        break;
                    case 3: //green
                        writeled(170,1);
                        writeled(169,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 25: //bol
        if (g[region].occupy_type != 2) {
                int coast = rand() % 2;
                coast = coast + 154;
                if (g[region].occupy_type == 1) writeled(coast,1); //TODO add coast support here
                if (g[region].occupy_type == 0) writeled(153,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(150,1);
                        writeled(151,1);
                        writeled(152,1);
                        break;
                    case 1: //red
                        writeled(150,1);
                        writeled(151,1);
                        break;
                    case 2: //blue
                        writeled(152,1);
                        writeled(150,1);
                        break;
                    case 3: //green
                        writeled(151,1);
                        writeled(152,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 26: //mal
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(172,1);
                if (g[region].occupy_type == 0) writeled(173,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(156,1);
                        writeled(157,1);
                        writeled(158,1);
                        break;
                    case 1: //red
                        writeled(156,1);
                        writeled(157,1);
                        break;
                    case 2: //blue
                        writeled(158,1);
                        writeled(156,1);
                        break;
                    case 3: //green
                        writeled(157,1);
                        writeled(158,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(174,1);
                    writeled(175,1);
                    writeled(159,1);
                    break;
                case 1: //red
                    writeled(159,1);
                    writeled(175,1);
                    break;
                case 2: //blue
                    writeled(174,1);
                    writeled(159,1);
                    break;
                case 3: //green
                    writeled(175,1);
                    writeled(174,1);
                    break;
                default:
                    break;
            }
            break;
        case 27: //ska
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(149,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(146,1);
                        writeled(147,1);
                        writeled(148,1);
                        break;
                    case 1: //red
                        writeled(146,1);
                        writeled(147,1);
                        break;
                    case 2: //blue
                        writeled(148,1);
                        writeled(146,1);
                        break;
                    case 3: //green
                        writeled(147,1);
                        writeled(148,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 28: //got
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(160,1);
                if (g[region].occupy_type == 0) writeled(161,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(165,1);
                        writeled(166,1);
                        writeled(167,1);
                        break;
                    case 1: //red
                        writeled(167,1);
                        writeled(163,1);
                        break;
                    case 2: //blue
                        writeled(165,1);
                        writeled(167,1);
                        break;
                    case 3: //green
                        writeled(163,1);
                        writeled(165,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(162,1);
                    writeled(163,1);
                    writeled(164,1);
                    break;
                case 1: //red
                    writeled(164,1);
                    writeled(163,1);
                    break;
                case 2: //blue
                    writeled(162,1);
                    writeled(164,1);
                    break;
                case 3: //green
                    writeled(163,1);
                    writeled(162,1);
                    break;
                default:
                    break;
            }
         break;
         case 29: //van
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(188,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(189,1);
                        writeled(190,1);
                        writeled(191,1);
                        break;
                    case 1: //red
                        writeled(191,1);
                        writeled(190,1);
                        break;
                    case 2: //blue
                        writeled(189,1);
                        writeled(191,1);
                        break;
                    case 3: //green
                        writeled(190,1);
                        writeled(189,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 30: //sil
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(183,1); 
                if (g[region].occupy_type == 0) writeled(184,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(185,1);
                        writeled(186,1);
                        writeled(187,1);
                        break;
                    case 1: //red
                        writeled(187,1);
                        writeled(186,1);
                        break;
                    case 2: //blue
                        writeled(185,1);
                        writeled(187,1);
                        break;
                    case 3: //green
                        writeled(186,1);
                        writeled(185,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 31: //sto
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(192,1);
                if (g[region].occupy_type == 0) writeled(176,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(178,1);
                        writeled(179,1);
                        writeled(177,1);
                        break;
                    case 1: //red
                        writeled(179,1);
                        writeled(178,1);
                        break;
                    case 2: //blue
                        writeled(177,1);
                        writeled(179,1);
                        break;
                    case 3: //green
                        writeled(178,1);
                        writeled(177,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(180,1);
                    writeled(181,1);
                    writeled(182,1);
                    break;
                case 1: //red
                    writeled(182,1);
                    writeled(181,1);
                    break;
                case 2: //blue
                    writeled(180,1);
                    writeled(182,1);
                    break;
                case 3: //green
                    writeled(181,1);
                    writeled(180,1);
                    break;
                default:
                    break;
            }
         break;
         case 32: //ala
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(200,1);
                if (g[region].occupy_type == 0) writeled(199,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(193,1);
                        writeled(194,1);
                        writeled(195,1);
                        break;
                    case 1: //red
                        writeled(193,1);
                        writeled(194,1);
                        break;
                    case 2: //blue
                        writeled(195,1);
                        writeled(193,1);
                        break;
                    case 3: //green
                        writeled(194,1);
                        writeled(195,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(196,1);
                    writeled(197,1);
                    writeled(198,1);
                    break;
                case 1: //red
                    writeled(196,1);
                    writeled(197,1);
                    break;
                case 2: //blue
                    writeled(198,1);
                    writeled(196,1);
                    break;
                case 3: //green
                    writeled(197,1);
                    writeled(198,1);
                    break;
                default:
                    break;
            }
         break;
        case 33: //sgo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(204,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(201,1);
                        writeled(202,1);
                        writeled(203,1);
                        break;
                    case 1: //red
                        writeled(201,1);
                        writeled(202,1);
                        break;
                    case 2: //blue
                        writeled(203,1);
                        writeled(201,1);
                        break;
                    case 3: //green
                        writeled(202,1);
                        writeled(203,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 34: //hel
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(212,1);
                if (g[region].occupy_type == 0) writeled(211,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(208,1);
                        writeled(209,1);
                        writeled(210,1);
                        break;
                    case 1: //red
                        writeled(208,1);
                        writeled(211,1);
                        break;
                    case 2: //blue
                        writeled(210,1);
                        writeled(208,1);
                        break;
                    case 3: //green
                        writeled(211,1);
                        writeled(210,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(205,1);
                    writeled(206,1);
                    writeled(207,1);
                    break;
                case 1: //red
                    writeled(205,1);
                    writeled(206,1);
                    break;
                case 2: //blue
                    writeled(207,1);
                    writeled(205,1);
                    break;
                case 3: //green
                    writeled(206,1);
                    writeled(207,1);
                    break;
                default:
                    break;
            }
        break;
        case 35: //gof
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(216,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(213,1);
                        writeled(214,1);
                        writeled(215,1);
                        break;
                    case 1: //red
                        writeled(213,1);
                        writeled(214,1);
                        break;
                    case 2: //blue
                        writeled(215,1);
                        writeled(213,1);
                        break;
                    case 3: //green
                        writeled(214,1);
                        writeled(215,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 36: //lla
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(220,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(217,1);
                        writeled(218,1);
                        writeled(219,1);
                        break;
                    case 1: //red
                        writeled(217,1);
                        writeled(218,1);
                        break;
                    case 2: //blue
                        writeled(219,1);
                        writeled(217,1);
                        break;
                    case 3: //green
                        writeled(218,1);
                        writeled(219,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 37: //kuy
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(238,1); 
                if (g[region].occupy_type == 0) writeled(239,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(221,1);
                        writeled(222,1);
                        writeled(223,1);
                        break;
                    case 1: //red
                        writeled(221,1);
                        writeled(222,1);
                        break;
                    case 2: //blue
                        writeled(223,1);
                        writeled(221,1);
                        break;
                    case 3: //green
                        writeled(222,1);
                        writeled(223,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 38: //kuo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(230,1);
                if (g[region].occupy_type == 0) writeled(231,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(235,1);
                        writeled(236,1);
                        writeled(237,1);
                        break;
                    case 1: //red
                        writeled(237,1);
                        writeled(236,1);
                        break;
                    case 2: //blue
                        writeled(235,1);
                        writeled(237,1);
                        break;
                    case 3: //green
                        writeled(236,1);
                        writeled(235,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(232,1);
                    writeled(233,1);
                    writeled(234,1);
                    break;
                case 1: //red
                    writeled(234,1);
                    writeled(233,1);
                    break;
                case 2: //blue
                    writeled(232,1);
                    writeled(234,1);
                    break;
                case 3: //green
                    writeled(233,1);
                    writeled(232,1);
                    break;
                default:
                    break;
            }
        break;
        case 39: //sai
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(225,1); 
                if (g[region].occupy_type == 0) writeled(226,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(227,1);
                        writeled(228,1);
                        writeled(229,1);
                        break;
                    case 1: //red
                        writeled(229,1);
                        writeled(228,1);
                        break;
                    case 2: //blue
                        writeled(227,1);
                        writeled(229,1);
                        break;
                    case 3: //green
                        writeled(228,1);
                        writeled(227,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 40: //vas
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(249,1);
                if (g[region].occupy_type == 0) writeled(250,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(252,1);
                        writeled(253,1);
                        writeled(251,1);
                        break;
                    case 1: //red
                        writeled(253,1);
                        writeled(252,1);
                        break;
                    case 2: //blue
                        writeled(251,1);
                        writeled(253,1);
                        break;
                    case 3: //green
                        writeled(252,1);
                        writeled(251,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(224,1);
                    writeled(254,1);
                    writeled(255,1);
                    break;
                case 1: //red
                    writeled(224,1);
                    writeled(255,1);
                    break;
                case 2: //blue
                    writeled(254,1);
                    writeled(224,1);
                    break;
                case 3: //green
                    writeled(255,1);
                    writeled(254,1);
                    break;
                default:
                    break;
            }
        break;
        case 41: //ngo
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(245,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(246,1);
                        writeled(247,1);
                        writeled(248,1);
                        break;
                    case 1: //red
                        writeled(248,1);
                        writeled(247,1);
                        break;
                    case 2: //blue
                        writeled(246,1);
                        writeled(248,1);
                        break;
                    case 3: //green
                        writeled(247,1);
                        writeled(246,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 42: //sun
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(256,1);
                if (g[region].occupy_type == 0) writeled(257,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(258,1);
                        writeled(240,1);
                        writeled(241,1);
                        break;
                    case 1: //red
                        writeled(241,1);
                        writeled(240,1);
                        break;
                    case 2: //blue
                        writeled(258,1);
                        writeled(241,1);
                        break;
                    case 3: //green
                        writeled(240,1);
                        writeled(258,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(242,1);
                    writeled(243,1);
                    writeled(244,1);
                    break;
                case 1: //red
                    writeled(244,1);
                    writeled(243,1);
                    break;
                case 2: //blue
                    writeled(242,1);
                    writeled(244,1);
                    break;
                case 3: //green
                    writeled(243,1);
                    writeled(242,1);
                    break;
                default:
                    break;
            }
        break;
        case 43: //stj
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(262,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(260,1);
                        writeled(261,1);
                        writeled(259,1);
                        break;
                    case 1: //red
                        writeled(259,1);
                        writeled(260,1);
                        break;
                    case 2: //blue
                        writeled(261,1);
                        writeled(259,1);
                        break;
                    case 3: //green
                        writeled(260,1);
                        writeled(261,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 44: //keb
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(267,1); 
                if (g[region].occupy_type == 0) writeled(266,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(263,1);
                        writeled(264,1);
                        writeled(265,1);
                        break;
                    case 1: //red
                        writeled(263,1);
                        writeled(264,1);
                        break;
                    case 2: //blue
                        writeled(265,1);
                        writeled(263,1);
                        break;
                    case 3: //green
                        writeled(264,1);
                        writeled(265,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 45: //tor
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(275,1);
                if (g[region].occupy_type == 0) writeled(274,1);
                switch (g[region].player) {
                    case 0: //white
                        writeled(268,1);
                        writeled(269,1);
                        writeled(270,1);
                        break;
                    case 1: //red
                        writeled(268,1);
                        writeled(269,1);
                        break;
                    case 2: //blue
                        writeled(270,1);
                        writeled(268,1);
                        break;
                    case 3: //green
                        writeled(269,1);
                        writeled(270,1);
                    break;
                    default:
                    break;
                }
            }
            switch (g[region].supply) {
                case 0: //white
                    writeled(271,1);
                    writeled(272,1);
                    writeled(273,1);
                    break;
                case 1: //red
                    writeled(271,1);
                    writeled(272,1);
                    break;
                case 2: //blue
                    writeled(273,1);
                    writeled(271,1);
                    break;
                case 3: //green
                    writeled(272,1);
                    writeled(273,1);
                    break;
                default:
                    break;
            }
        break;
        case 46: //oul
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 1) writeled(280,1); 
                if (g[region].occupy_type == 0) writeled(279,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(276,1);
                        writeled(278,1);
                        writeled(277,1);
                        break;
                    case 1: //red
                        writeled(276,1);
                        writeled(277,1);
                        break;
                    case 2: //blue
                        writeled(278,1);
                        writeled(276,1);
                        break;
                    case 3: //green
                        writeled(277,1);
                        writeled(278,1);
                    break;
                    default:
                    break;
                }
            }
        break;
        case 47: //ima
            if (g[region].occupy_type != 2) {
                if (g[region].occupy_type == 0) writeled(284,1); 
                switch (g[region].player) {
                    case 0: //white
                        writeled(281,1);
                        writeled(282,1);
                        writeled(283,1);
                        break;
                    case 1: //red
                        writeled(281,1);
                        writeled(282,1);
                        break;
                    case 2: //blue
                        writeled(283,1);
                        writeled(281,1);
                        break;
                    case 3: //green
                        writeled(282,1);
                        writeled(283,1);
                    break;
                    default:
                    break;
                }
            }
        break;
    writeout();
    }
}



