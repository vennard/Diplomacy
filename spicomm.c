/*
 SPI testing utility (using spidev driver)

 Copyright (c) 2007  MontaVista Software, Inc.
 Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License.

 Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include "region.h"
#include "order.h"
#include "include.h"

//User Definitions
#define TIMEOUT 400000  //in clock cycles 
//#define TIMEOUT 40000  //in clock cycles 
//#define TIMEOUT 25000  //in clock cycles 

/*
 *CC1101 Transfer Details (from the datasheet)
 *64-byte TX & RX FIFOs
 * Burst access: Expects one header and then continuous bytes until terminated access
 *               by setting CSn high. Otherwise expects header before every byte of data.
 * Header Byte access to FIFO is below:
 */
#define TX_BYTE                      0x3F
#define TX_BURST                     0x7F
#define RX_BYTE                      0xBF
#define RX_BURST                     0xFF
///////////////////////////////////////////////////////////////////////////////////
// CC1101 WRITE MASKS
#define WRITE_BURST                  0x40
#define READ_SINGLE                  0x80
#define READ_BURST                   0xC0

#define CONFIG_REG                   0x80
#define STATUS_REG                   0xC0
///////////////////////////////////////////////////////////////////////////////////////
// CC1101 CONFIG REGSITER
#define _IOCFG2                      0x00 // GDO2 output pin configuration
#define _IOCFG1                      0x01 // GDO1 output pin configuration
#define _IOCFG0                      0x02 // GDO0 output pin configuration
#define _FIFOTHR                     0x03 // RX FIFO and TX FIFO thresholds
#define _SYNC1                       0x04 // Sync word, high INT8U
#define _SYNC0                       0x05 // Sync word, low INT8U
#define _PKTLEN                      0x06 // Packet length
#define _PKTCTRL1                    0x07 // Packet automation control
#define _PKTCTRL0                    0x08 // Packet automation control
#define _ADDR                        0x09 // Device address
#define _CHANNR                      0x0A // Channel number
#define _FSCTRL1                     0x0B // Frequency synthesizer control
#define _FSCTRL0                     0x0C // Frequency synthesizer control
#define _FREQ2                       0x0D // Frequency control word, high INT8U
#define _FREQ1                       0x0E // Frequency control word, middle INT8U
#define _FREQ0                       0x0F // Frequency control word, low INT8U
#define _MDMCFG4                     0x10 // Modem configuration
#define _MDMCFG3                     0x11 // Modem configuration
#define _MDMCFG2                     0x12 // Modem configuration
#define _MDMCFG1                     0x13 // Modem configuration
#define _MDMCFG0                     0x14 // Modem configuration
#define _DEVIATN                     0x15 // Modem deviation setting
#define _MCSM2                       0x16 // Main Radio Control State Machine configuration
#define _MCSM1                       0x17 // Main Radio Control State Machine configuration
#define _MCSM0                       0x18 // Main Radio Control State Machine configuration
#define _FOCCFG                      0x19 // Frequency Offset Compensation configuration
#define _BSCFG                       0x1A // Bit Synchronization configuration
#define _AGCCTRL2                    0x1B // AGC control
#define _AGCCTRL1                    0x1C // AGC control
#define _AGCCTRL0                    0x1D // AGC control
#define _WOREVT1                     0x1E // High INT8U Event 0 timeout
#define _WOREVT0                     0x1F // Low INT8U Event 0 timeout
#define _WORCTRL                     0x20 // Wake On Radio control
#define _FREND1                      0x21 // Front end RX configuration
#define _FREND0                      0x22 // Front end TX configuration
#define _FSCAL3                      0x23 // Frequency synthesizer calibration
#define _FSCAL2                      0x24 // Frequency synthesizer calibration
#define _FSCAL1                      0x25 // Frequency synthesizer calibration
#define _FSCAL0                      0x26 // Frequency synthesizer calibration
#define _RCCTRL1                     0x27 // RC oscillator configuration
#define _RCCTRL0                     0x28 // RC oscillator configuration
#define _FSTEST                      0x29 // Frequency synthesizer calibration control
#define _PTEST                       0x2A // Production test
#define _AGCTEST                     0x2B // AGC test
#define _TEST2                       0x2C // Various test settings
#define _TEST1                       0x2D // Various test settings
#define _TEST0                       0x2E // Various test settings

//CC1101 Strobe commands
#define _SRES                        0x30 // Reset chip.
#define _SFSTXON                     0x31 // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                          // If in RX/TX: Go to a wait state where only the synthesizer is
                                          // running (for quick RX / TX turnaround).
#define _SXOFF                       0x32 // Turn off crystal oscillator.
#define _SCAL                        0x33 // Calibrate frequency synthesizer and turn it off
                                          // (enables quick start).
#define _SRX                         0x34 // Enable RX. Perform calibration first if coming from IDLE and
                                          // MCSM0.FS_AUTOCAL=1.
#define _STX                         0x35 // In IDLE state: Enable TX. Perform calibration first if
                                          // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                          // Only go to TX if channel is clear.
#define _SIDLE                       0x36 // Exit RX / TX, turn off frequency synthesizer and exit
                                                // Wake-On-Radio mode if applicable.
#define _SAFC                        0x37 // Perform AFC adjustment of the frequency synthesizer
#define _SWOR                        0x38 // Start automatic RX polling sequence (Wakeon-Radio)
#define _SPWD                        0x39 // Enter power down mode when CSn goes high.
#define _SFRX                        0x3A // Flush the RX FIFO buffer.
#define _SFTX                        0x3B // Flush the TX FIFO buffer.
#define _SWORRST                     0x3C // Reset real time clock.
#define _SNOP                        0x3D // No operation. May be used to pad strobe commands to two
                                          // INT8Us for simpler software.

///////////////////////////////////////////////////////////////////////////////////////
//CC1101 Status Registers
#define _PARTNUM                     0x30
#define _VERSION                     0x31
#define _FREQEST                     0x32
#define _LQI                         0x33
#define _RSSI                        0x34
#define _MARCSTATE                   0x35
#define _WORTIME1                    0x36
#define _WORTIME0                    0x37
#define _PKTSTATUS                   0x38
#define _VCO_VC_DAC                  0x39
#define _TXBYTES                     0x3A
#define _RXBYTES                     0x3B
#define _RCCTRL1_STATUS              0x3C
#define _RCCTRL0_STATUS              0x3D

//////////////////////////////////////////////////////////////////////////////////////
// Definitions to support burst/single access:
#define CRC_OK                       0x80
#define RSSI                         0
#define LQI                          1
#define BYTES_IN_RXFIFO              0x7F

// Definitions for chip status
#define CHIP_RDY                     0x80
#define CHIP_STATE_MASK              0x70
#define CHIP_STATE_IDLE              0x00
#define CHIP_STATE_RX                0x10
#define CHIP_STATE_TX                0x20
#define CHIP_STATE_FSTON             0x30
#define CHIP_STATE_CALIBRATE         0x40
#define CHIP_STATE_SETTLING          0x50
#define CHIP_STATE_RXFIFO_OVERFLOW   0x60
#define CHIP_STATE_TXFIFO_UNDERFLOW  0x70
#define FIFO_BYTES_MASK              0x0F

//CC1101 PATABLE,TXFIFO,RXFIFO
#define _PATABLE                     0x3E
#define _TXFIFO                      0x3F
#define _RXFIFO                      0x3F
#define _SPI_WRITE_MASK              0x80



static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
//static uint32_t speed = 900000;
static uint32_t speed = 1000000;
static uint16_t delay = 100;

static void pabort(const char *s) {
  perror(s);
  abort();
}

void changemode(int mode);
uint8_t send(int len, uint8_t *in);  
void rxd(); 
void rxdata(int len); 
uint8_t bitswiz(uint8_t in);

//Address, Data
//38 elements
uint8_t config[38][2] = { 
    { _FSCTRL1, 0x08 },
    //{ _FSCTRL1, 0x06 },
    { _FSCTRL0, 0x00 },
    { _FREQ2, 0x23   },
    { _FREQ1, 0x31   },
    { _FREQ0, 0x3B   },
    { _MDMCFG4, 0xCA }, //prolly needed on both
    //{ _MDMCFG4, 0xF5 },
    { _MDMCFG3, 0x83 },
    { _MDMCFG2, 0x93 }, //Enable digital DC blocking
    //{ _MDMCFG2, 0x13 },
    { _MDMCFG1, 0x22 }, //FEC_EN
    //{ _MDMCFG1, 0xa2 },
    { _MDMCFG0, 0xF8 },
    { _CHANNR, 0x00  },
    { _DEVIATN, 0x34 }, //expected frequency deviation -- not needed on both
    //{ _DEVIATN, 0x31 },
    { _FREND1, 0x56  },
    { _FREND0, 0x10  },
    { _MCSM0, 0x18   },
    { _FOCCFG, 0x16  },
    { _BSCFG, 0x6C   },  
    { _AGCCTRL2, 0x43 },
    { _AGCCTRL1, 0x40 },
    { _FSCAL3, 0x91  },  //calibration stuff and chargepump
    //{ _FSCAL3, 0xE9  }, 
    { _FSCAL2, 0x29  },  //vco choices and calibration results
    //{ _FSCAL2, 0x2A  }, 
    { _FSCAL1, 0x2A  },  //coarse tuning
    //{ _FSCAL1, 0x00  }, 
    { _FSCAL0, 0x00  },  //calibration control
    //{ _FSCAL0, 0x1F  }, 
    { _FSTEST, 0x59  }, 
    { _TEST2, 0x81   },
    { _TEST1, 0x35   },
    { _TEST0, 0x09   },
    { _FIFOTHR, 0x47 },
    { _IOCFG2, 0x29  },
    { _IOCFG0, 0x06  },
    //{ _PKTCTRL1, 0x0c},
    { _PKTCTRL1, 0x04},
    { _PKTCTRL0, 0x04},
    //{ _PKTCTRL0, 0x05},
    { _ADDR, 0x00    },
    { _PKTLEN, 0x07  },
    { _WORCTRL, 0xFB },
    { _SYNC1, 0xDD   },
    { _SYNC0, 0xDD   },
};

uint8_t readingdata[64];
unsigned char ackcount[4];
order_t inputorders[256][4];
int fd, numinc;

void testfunc() {
printf("CALLED TESTING FUNCTION");
}

//waits for acknowledge(s) from controller(s)
//type 0 - 3 = controller to wait for
//type 4 = wait for ack from all controllers
//type 5 = request rx mode
//return - 0 if successfully got acknowledge(s)
//         1 - timed out! retry comm and call again
int ackwait(int type, int messageid) {
    clock_t tstart = clock();
    int i;
    int count = 0;
    int check[4] = {0,0,0,0};
    switch(type) {
        case 0:
        case 1:
        case 2:
        case 3: 
            while (count < 1) {
                rxd(); //read 2 byte acknowledge
                if ((readingdata[1] == 0x2C)&&(readingdata[0] == type)&&(readingdata[2] == messageid)) {
                    printf("Found successful ack!\r\n");
		            //usleep(2500);
                    count++;
            	    ackcount[type]++;//Increment acknowledge count
                    if (ackcount[type] > 0x7f) ackcount[type] = 0;
                    return 0;
                } else {
		            clock_t now = clock();
		            int telapsed = now - tstart;
		            if ((telapsed <= 0)||(telapsed > TIMEOUT)) return 1; 
                }
            }
            break;
        case 4:
            while (count < 4) { //TODO needs work
                rxdata(7); //read 2 byte acknowledge
                if (readingdata[2] == 0xAC) { //found valid acknowledge
		            int checkval = (int) readingdata[1];
                    if ((checkval < 0)||(checkval > 3)) {
                        printf("ERROR: Invalid source address received!!!\r\n");
                    } else {
                        printf("         Found valid acknowledge from id %i!\r\n",readingdata[1]);
                        check[readingdata[1]] = 1;
                    }
                }
                //count acknowledges 
                count = 0;
                for (i = 0;i < 4;i++) count += check[i]; 
                //check for timeout
                int telapsed = difftime(time(NULL), tstart);
                if (telapsed > TIMEOUT) { //timeout 
                    printf("COM ERROR: Timed out after %i seconds waiting for acks!\r\n",TIMEOUT);
                    return 1;
                }
            }
            printf("Found all 4 acknowledges!!! Continuing...\r\n");
            for (i = 0;i < 4;i++) {
                ackcount[i]++;
                if (ackcount[i] > 0x7f) ackcount[type] = 0;
            }
            break;
        case 5:   //TODO remove global calls to ackcount & support 4 controllers
	        while (count < 1) {
                rxd(); 
                printf("RX'ed in ackwait(5): ");
                int i; 
                for (i = 0;i < 7;i++) printf("0x%x ",readingdata[i]);
                printf("\r\n");
                //Exit once stop message is recieved 
		        if ((readingdata[0] == 0)&&(readingdata[1] == ackcount[0])&&(readingdata[2] == 0x5E)) {
		            printf("Saw end transmission byte!!!");
		            count = 1;
            	    ackcount[0]++;//Increment acknowledge count
                    if (ackcount[0] > 0x7f) ackcount[0] = 0;
		            return 0;
                // save order if valid request is recieved
                } else if ((readingdata[1] == ackcount[0])&&(readingdata[0] == 0)) {
                    printf("Got valid order! saving off\r\n");
		            //usleep(2500);
		            //changemode(0); 
		            //uint8_t clearbuf = _SFRX;
		            //send(1, &clearbuf);
		            order_t no;
    	            no.player = 0;
                    no.country = readingdata[2]; 
                    no.order = readingdata[3]; 
                    no.type = readingdata[4]; 
                    no.tcountry = readingdata[5]; 
                    no.scountry = readingdata[6]; 
		            o[numinc] = no;
                    printf("ACK was readingdata[2] - %x vs ackcount - %x\r\n",readingdata[2],ackcount[0]);
                    printf("Order %i was: player %x, country %x, order %x,",numinc, no.player,no.country, no.order);
                    printf(" unit type %x, tcountry %x (from rx %x),",no.type, no.tcountry,readingdata[6]);
                    printf(" scountry %x (from rx %x)!\r\n",no.scountry,readingdata[7]);
		            numinc++;
            	    ackcount[0]++;//Increment acknowledge count
		            return 1; //keep requesting packets until you see ending byte
                } else {
		            clock_t now = clock();
		            int telapsed = now - tstart;
		            if ((telapsed < 0)||(telapsed > TIMEOUT)) return 1; 
                }
            }
	    break;
        default:
            printf("Invalid ackwait call!\r\n");
            return 0;
    }
    return 0;
}
//Simple read of len bytes off of the RX fifo
//writes to readingdata[]
void rxdata(int len) {
    int i, ret;
    uint8_t tx[len+1]; 
	int done = 0;
	while (!done) {
    	changemode(0);
    	tx[0] = _PKTSTATUS | 0x80;
    	tx[1] = _SNOP;
		uint8_t pck = send(2, tx);
		uint8_t gdo2 = pck & 0x04; //gdo2 configured to hold CRC check result
		tx[0] = _RXBYTES | 0x80;
    	uint8_t result = send(2, tx);
		//result = result & 0x7f;
		if ((gdo2 == 0x04)&&(result > 6)) { //CRC check ok
    	    changemode(2); //put in rx 
    	    if (len < 1) exit(1);
    		if (len < 2) tx[0] = RX_BYTE;
        	if (len > 1) tx[0] = RX_BURST;
    		struct spi_ioc_transfer fr = {
        		.tx_buf = (unsigned long) tx,
        		.rx_buf = (unsigned long) readingdata,
        		.len = len+1, 
        		.delay_usecs = delay,
        		.speed_hz = speed,
        		.bits_per_word = bits,
    		};
    		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
    		if (ret < 1) pabort("can't send spi message");
     //       usleep(2500);
    		//printf("RX: ");
    		//for (i = 0;i < len+1;i++) printf("0x%x ",readingdata[i]);
    		//printf("\r\n");
              //check status byte
    		uint8_t check = readingdata[0] & 0x70;
    		if (check != 0x10) printf("WARNING: READ STATUS RETURN GAVE NON-RX VALUE: 0x%x!!!!\r\n",readingdata[0]);
		    done = 1;
        }
    } 
}

//read in 7 bytes from the connected controller
void rxd() {
    int i, ret;
    uint8_t tx = 0x01;
    uint8_t in;
    printf("\r\n");
    for(i = 0;i < 7;i++) {
        usleep(2500);
        struct spi_ioc_transfer fr = {
       		.tx_buf = (unsigned long) &tx,
        	.rx_buf = (unsigned long) &in,
        	.len = 1, 
        	.delay_usecs = delay,
        	.speed_hz = speed,
        	.bits_per_word = bits,
    	};
    	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
    	if (ret < 1) pabort("can't send spi message");
        uint8_t out = bitswiz(in);
        readingdata[i] = out; //save off data
        if (out == 0x3B) {
            printf(".");
            //sleep(2);
            //usleep(2500);
            usleep(25000);
            i--;
        }
        //printf("RX: 0x%x \r\n",out);
        //printf("hit enter to continue ");
        //int x = getchar();
    }
    printf("\r\n");
    //printf("RX: ");
    //for (i = 0;i < 7;i++) printf("0x%x ", readingdata[i]);
    //printf(" received... \r\n");
}

//uses 4th controller as sending device
void txd(int acktype, uint8_t *in) {
    int i, ret;
    int ackd = 0;
    uint8_t tx[7]; 
    uint8_t rx[7]; 
    //printf("Sending TX: ");
    for (i = 0;i < 7;i++) {
        tx[i] = in[i]; //sending 7 bytes
      //  printf("0x%x ",tx[i]);
    }
    //printf("\r\n");
    while (ackd == 0) {
        for(i = 0;i < 63;i++) readingdata[i] = 0; //clear local buffer
	    if (acktype == 5) tx[1] = ackcount[tx[2]]; //increment ack call for rxorders phase 
        //Send 8 bytes out via spi
        for (i = 0; i < 7;i++) {
     //       printf("ready to send 0x%x!\r\n",tx[i]);
            uint8_t ttx = tx[i];
            struct spi_ioc_transfer fr = {
                .tx_buf = (unsigned long) &ttx,
                .rx_buf = (unsigned long) rx,
                .len = 1,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
            };
            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
            if (ret < 1) pabort("can't send spi message");
            uint8_t out = bitswiz(rx[0]);
      //      printf("got back 0x%x!\r\n",out);
            if ((out == 0x55)||(out == 0x54)) {
           //     printf("success!\r\n");
                usleep(10);
            } else {
                i--;
                //usleep(25000);
            //    printf("Found invalid response, resending byte!\r\n");
            }
        }
        printf("Sent 7 bytes successfully! Now waiting for ack\r\n");
        if (ackwait(acktype, tx[1]) == 1) {
            printf("ERROR! Timed out, resending!\r\n");
        } else {
        //    printf("SUCCESS! TX Data: ");
         //   for (i = 0;i < 7;i++) printf("0x%x ",tx[i]);
          //  printf(" sent and acknowledged.\r\n");
            ackd = 1;
        }
    }
}


//transfers data via radio
//default is 7 byte transfer burst (Not including header for spi)
//acktype is needed acknowledge address (4 for all) 5 for request data mode
//returns 0 on success, 1 otherwise
void txdata(int acktype, uint8_t *in) {
    int i, ret;
    int ackd = 0;
    uint8_t tx[8]; 
    uint8_t rx[8]; 
    for (i = 1;i < 8;i++) tx[i] = in[i];
    for(i = 0;i < 63;i++) readingdata[i] = 0; //clear local buffer
    numinc = 0;
    while (ackd == 0) {
        tx[0] = _SFRX;
        changemode(0);
        send(1,tx);
    	tx[0] = TX_BURST;
	    if (acktype == 5) tx[2] = ackcount[tx[3]]; //increment ack call for rxorders phase 
        changemode(1); //put in tx mode
        struct spi_ioc_transfer fr = {
            .tx_buf = (unsigned long) tx,
            .rx_buf = (unsigned long) rx,
            .len = 8,
            .delay_usecs = delay,
            .speed_hz = speed,
            .bits_per_word = bits,
        };
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
        if (ret < 1) pabort("can't send spi message");
	    usleep(2500);	//ABSOLUTELY NECESSARY DO NOT REMOVE!!!
        uint8_t check = rx[0] & 0x70;
        if (check != 0x20) {
            printf("ERROR: CC1101 is not in tx mode!\r\n");
            changemode(0); //send back to idle
        } else if (ackwait(acktype,tx[2]) == 1) {
            printf("ERROR: Timed out! Resending...\r\n");
        } else {
            printf("SUCCESS! TX Data: ");
            for (i = 1;i < 8;i++) printf("0x%x ",tx[i]);
            printf(" sent and acknowledged.\r\n");
            ackd = 1;
        }
    }
}

//Sends data over spi
//len = number of bytes to be sent via spi
//returns last rx byte received from cc1101
uint8_t send(int len, uint8_t *in) { 
    if (len < 0) return -1;
    int i, ret;
    uint8_t rx[len];
    struct spi_ioc_transfer fr = {
        .tx_buf = (unsigned long) in,
        .rx_buf = (unsigned long) rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
    if (ret < 1) pabort("can't send spi message");
    //printf("RX from send = 0x%x 0x%x\r\n",rx[0],rx[1]);
    return rx[len-1];  
} 

//changes endianness for byte
uint8_t bitswiz(uint8_t in) {
    uint8_t temp[8];
    int i;
    temp[0] = (in & 0x01);
    temp[0] = temp[0] << 7; 
    temp[1] = (in & 0x02);
    temp[1] = temp[1] << 5;
    temp[2] = (in & 0x04);
    temp[2] = temp[2] << 3;
    temp[3] = (in & 0x08);
    temp[3] = temp[3] << 1;
    temp[4] = (in & 0x10);
    temp[4] = temp[4] >> 1;
    temp[5] = (in & 0x20);
    temp[5] = temp[5] >> 3;
    temp[6] = (in & 0x40);
    temp[6] = temp[6] >> 5;
    temp[7] = (in & 0x80);
    temp[7] = temp[7] >> 7;
    uint8_t out = temp[0] | temp[1] | temp[2] | temp[3] | temp[4] | temp[5] | temp[6] | temp[7];
    out = out >> 1;
    out = out & 0x7f;
    return out;
}

void testcontroller() {
    setupspi();
    printf("Launching test program\r\n");
    int i, ret;
    uint8_t tx[7];
    uint8_t rx[7];
     //1. Send Phase Type (Key & Data) 2 bytes
    printf("1. Sending phase type:\r\n");
    tx[0] = 0x6D; //sender addr
    tx[1] = 0x02; //using controller zero as sync ack 
    //tx[1] = ackcount[0]; //using controller zero as sync ack 
    tx[2] = 0x08; //phase type key
    tx[3] = 0x01; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
    tx[4] = 0x23; 
    tx[5] = 0x45;
    tx[6] = 0x67;

    //tx[0] = 0x6D; //sender addr
    //tx[1] = 0x01; //using controller zero as sync ack 
    //tx[2] = 0x99; //phase type key
    //tx[3] = 0x21; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
    //tx[4] = 0x69; 
    //tx[5] = 0x45;

    int buf;
    for (i = 0; i < 7;i++) {
        printf("ready to send 0x%x!\r\n",tx[i]);
        buf = getchar();
        //sleep(5);
        uint8_t ttx = tx[i];
        struct spi_ioc_transfer fr = {
            .tx_buf = (unsigned long) &ttx,
            .rx_buf = (unsigned long) rx,
            .len = 1,
            .delay_usecs = delay,
            .speed_hz = speed,
            .bits_per_word = bits,
        };
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
        if (ret < 1) pabort("can't send spi message");
        uint8_t out = bitswiz(rx[0]);
        printf("got back 0x%x!\r\n",out);
        if ((out == 0x55)||(out == 0x54)) {
            printf("success!\r\n");
        } else {
           // i--;
            sleep(1);
            printf("Found invalid response, resending byte!\r\n");
        }
    }
    //check response to make sure controller is in the correct state
    printf("Done with testing loop!\r\n");
}

//Write all data necessary to configure the CC1101
void configurespi() {
    printf("Setting up CC1101...\r\n");
    int i;
    uint8_t tx[2];
    uint8_t check;
    fd = open(device, O_RDWR);
    if (fd < 0) pabort("can't open device");
    setupspi();
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

    tx[0] = 0x30; 
    send(1, &tx[0]);  //reset chip 
    printf("reset!\r\n");
    changemode(0); //put in idle
    printf("Configuring... \r\n");
    for (i = 0;i < 4;i++) ackcount[i] = 0; //set all ack counts to zero 
    for (i = 0;i < 38;i++) {
        printf("%i: ",i);
        tx[0] = config[i][0]; 
        tx[1] = config[i][1];
        send(2, tx);
	    usleep(10);
        check = config[i][0] | 0x80;
        tx[0] = check;
        tx[1] = 0x00;
        check = send(2, tx);
        printf(" set 0x%x...",check);
        if (check != config[i][1]) {
            printf(" validation FAILED! retrying...\r\n"); 
            sleep(1);
            i--;
        } else {
            printf(" success!\r\n");
        }
    }
    changemode(0); //put in idle
    tx[0] = _SFTX;
    send(1,tx);
    printf("Cleared TX Buffer!\r\n");
    tx[0] = _SFRX;
    send(1,tx);
    printf("Cleared RX Buffer!\r\n");
    printf("---Completed CC1101 configuration successfully!---\r\n\r\n");
}

void setupspi() {
    fd = open(device, O_RDWR);
    if (fd < 0) pabort("can't open device");

    int ret;
    //spi mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    pabort("can't set spi mode");
    
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
    pabort("can't get spi mode");
    
    //bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    pabort("can't set bits per word");
    
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
    pabort("can't get bits per word");
    
    //max speed hz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    pabort("can't set max speed hz");
    
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    pabort("can't get max speed hz");
}


//mode = 0 then going to IDLE mode
//mode = 1 then going to TX mode
//mode = 2 then going to RX mode
void changemode(int mode) {
    //send nop to check status
    uint8_t state, check, rdy;
    uint8_t out;
    check = -1;
    switch (mode) {
        case 1: 
            state = CHIP_STATE_TX;
            //printf("Sending CC1101 to TX state.\r\n");
            break;
        case 2: 
            state = CHIP_STATE_RX;
            //printf("Sending CC1101 to RX state.\r\n");
            break;
        default:
            state = CHIP_STATE_IDLE;
            //printf("Sending CC1101 to IDLE state.\r\n");
            break;
    }
    out = _SNOP; 
    check = send(1, &out);
    rdy = check & 0x80; 
    check = check & 0x70;
    while(state != check) {
        if (rdy == 0x80) {
            //printf("Chip is NOT READY. Waiting for power/crystal to settle.\r\n");
        } else {
            switch (check) {
                case CHIP_STATE_SETTLING:
                    //printf("Chip is settling... waiting\r\n");
                    break;
                case CHIP_STATE_CALIBRATE:
                    //printf("Chip is calibrating... waiting\r\n");
                    break;
                case CHIP_STATE_TXFIFO_UNDERFLOW:
                    //printf("TX underflow detected, sending SFTX\r\n");
                    out = _SFTX;
                    send(1, &out);
                    break;
                case CHIP_STATE_RXFIFO_OVERFLOW:
                    //printf("RX overflow detected, sending SFRX\r\n");
                    out = _SFRX;
                    send(1, &out);
                    break;
                case CHIP_STATE_FSTON: 
                    //printf("Fast TX ready state detected, sending SIDLE\r\n");
                    out = _SIDLE;
                    send(1, &out);
                    break;
                case CHIP_STATE_RX: 
                    //printf("RX state detected, sending SIDLE\r\n");
                    out = _SIDLE;
                    send(1, &out);
                    break;
                case CHIP_STATE_TX: 
                    //printf("TX state detected, sending SIDLE\r\n");
                    out = _SIDLE;
                    send(1, &out);
                case CHIP_STATE_IDLE:
                    //printf("IDLE state detected, ");
                    if (state == CHIP_STATE_TX) {
                        //printf("sending to tx state\r\n");      
                        out = _STX;
                        send(1, &out);
                    } else if (state == CHIP_STATE_RX) {
                        //printf("sending to rx state\r\n");      
                        out = _SRX;
                        send(1, &out);
                    } else {
                        //printf("continuing\r\n");      
                    }
                    break;
                default:
                    break;
            }
        }
        usleep(2500);
        //printf("Rechecking... ");
        out = _SNOP; 
        check = send(1, &out);
        rdy = check & 0x80; 
        check = check & 0x70;
    }
    //printf("found correct state! continuing...\r\n");
}


//sends phase type and region data
//phasetype- 0 normal, 1 retreat/disband, 2 supply
//ptr will point to game (region) data array
void tx_phase_start(int phasetype) {
    uint8_t tx[64];
    uint8_t rx[64];
    int roundtime, i;
    int players = 0; //TODO change to 4 for actual game play
    //set round specific details
    switch(phasetype) {
        case 1:
            printf("Starting timer and region data transmission for retreat phase!\r\n");
            roundtime = RETREAT_TIME;
            break;
        case 2:
            printf("Starting timer and region data transmission for supply phase!\r\n");
            roundtime = SUPPLY_TIME;
            break;
        default:
            printf("Starting timer and region data transmission for orders phase!\r\n");
            roundtime = ORDERS_TIME;
            break;
    }

    //1. Start timer for the round
    settmr(roundtime); 
    starttmr();
    changemode(0); //put in idle
	tx[0] = _SFRX;
	send(1,tx); //flush rx buffer

    //1. Send Phase Type (Key & Data) 2 bytes
    printf("1. Sending phase type:\r\n");
    tx[1] = 0xED; //sender addr
    tx[2] = ackcount[0]; //using controller zero as sync ack 
    tx[3] = 0x88; //phase type key
    tx[4] = phasetype; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
    tx[5] = 0x00; 
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(players, tx); //wait for ack from each controller

    //2. Send ONLY OCCUPIED region data 0 -> 47 (Owner & Unit_type) 2 bytes
    printf("2. Sending occupied region data:\r\n");
    for (i = 0;i < 48;i++) {
        if (g[i].occupy_type != 2) { //country is occupied
            tx[1] = 0xED;
            tx[2] = ackcount[0]; 
            tx[3] = (uint8_t)g[i].player;
            tx[4] = (uint8_t)g[i].occupy_type;
            tx[5] = 0x00; 
            tx[6] = 0x00;
            tx[7] = 0x00;
            printf("%i: player - %i, type - %i\r\n",i,tx[3],tx[4]);
            txdata(players, tx);
            usleep(2500); //TODO hopefully get to remove
        } else {
            printf("%i: skipped!\r\n",i,tx[3],tx[4]);
        }
    }

    //3. Wait for round to end before continuing
    printf("Waiting for the round to finish...\r\n");
    while (checktmr() == 0);
}

//Gets all order data from controllers
//roundtype - 0 normal, 1 retreat/disband, 2 supply
//saves orders in inputorders[][] TODO will have to put in global o[]
//returns number of new orders
int rx_orders_start(int roundtype) {
    int i;
    uint8_t tx[64];
    int players = 0; //should be 4 for final operation

    //1. Send out lock signal to indicate end of round
    tx[1] = 0xED;
    tx[2] = ackcount[0]; 
    tx[3] = 0xBF; //round end signal
    tx[4] = 0x00;
    tx[5] = 0x00; 
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(players, tx);

    //2. Start polling to request orders from controllers
    for (i = 0;i < 1;i++) { //change to loop < 4 TODO
        changemode(0);
        tx[0] = _SFRX; //flush rx buffer
        send(1, tx);

        //2a. Send out player id data request
        tx[1] = 0xED; 
        tx[2] = ackcount[i]; //splits to send seperate acknowledge count
        tx[3] = (uint8_t) i; //player id
        tx[4] = 0x00;
        tx[5] = 0x00; 
        tx[6] = 0x00;
        tx[7] = 0x00;
        txdata(5, tx); //rx request mode

        //clear acknowledge counts
        for (i = 0;i < 4;i++) ackcount[i] = 0;
        printf("Done getting order data... launching arbirator\r\n");
        //TODO add call to arbitrator passing it inputorders[][]
        sleep(5);
        printf("End of Transmission!!!\r\n");
    }

    //TODO copy from below
    return 0;
}

void demo(void) {
    char bf[1];
    int country;
    printf("starting demo...\r\n");
    uint8_t tx[64];
    uint8_t rx[64];

    //1. prompt for country
    printf("Enter country you would like to control:");
    char buf[10];
    int good = 0;
    while(!good) {
        scanf("%s",buf);
        country = atoi(buf);
        if ((country < 0)||(country > 47)) { 
            printf("Invalid entry! Please try again.\r\n");
        } else {
            printf("Ok I'll see what I can do with country %i!\r\n",country);
            good = 1;
        }
    }
    printf("Sending selected data to controller!\r\n");
    //configurespi();

    //2. Send Phase Type (Key & Data) 2 bytes
    printf("2. Sending phase type:\r\n");
    tx[0] = 0x6D; //sender addr
    tx[1] = ackcount[0]; 
    tx[2] = 0x08; //phase type key
    tx[3] = 0x00; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
    tx[4] = 0x00; //N/A
    tx[5] = 0x00;
    tx[6] = 0x00;
    txd(0, tx);


    //3. Send out country data
    printf("3. sending country data:\r\n");
    tx[0] = 0x6D; //sender addr
    tx[1] = ackcount[0]; 
    tx[2] = (uint8_t) g[country].player;
    tx[3] = (uint8_t) g[country].occupy_type;
    tx[4] = (uint8_t) country; 
    tx[5] = 0x00;
    tx[6] = 0x00;
    txd(0, tx);

    //4. wait 1 minute turn period
    printf("Starting wait period of 1 min...");
    settmr(1);
    starttmr();
    while (checktmr() == 0); //spin on timer status
    printf("Timer done!\r\n");

    scanf("%s",bf);

    //5. Send lock to controller
    printf("Sending lock signal to controller!\r\n");
    tx[0] = 0x6D;
    tx[1] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
    tx[2] = 0x3F; //round end signal
    tx[3] = 0x00;
    tx[4] = 0x00; //N/A
    tx[5] = 0x00;
    tx[6] = 0x00;
    txd(0, tx);

    //6. Zedboard polls controller
    printf("Sending polling signal to controller!\r\n");
    tx[0] = 0x6D;
    tx[1] = ackcount[0]; //splits to send seperate acknowledge count
    tx[2] = 0; //player id
    tx[3] = 0x77;
    tx[4] = 0x00; 
    tx[5] = 0x00;
    tx[6] = 0x00;
    txd(5, tx); //get order data back

        //8. Send order data to arbitrator
    numO = numinc;
    arbitor();
}

void runspi(void) {
    printf("Called runspi!\r\n"); 
    //configurespi();
    setupspi();

    //Transmit test game data
    uint8_t tx[64];
    uint8_t rx[64];
    while(1) {
        printf("\r\nStart of Transmission!!!\r\n");

        //1. Send Phase Type (Key & Data) 2 bytes
        printf("1. Sending phase type:\r\n");
        tx[0] = 0x6D; //sender addr
        tx[1] = ackcount[0]; 
        tx[2] = 0x08; //phase type key
        tx[3] = 0x00; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
        tx[4] = 0x00; //N/A
        tx[5] = 0x00;
        tx[6] = 0x00;
        //txdata(4, tx); //TODO needs this
        txd(0, tx);
	
       // printf("hit enter to continue ");

        //2. Send region data 0 -> 47 (Owner & Unit_type) 2 bytes
        int i;
        printf("2. Sending region data:\r\n");
        for (i = 0;i < 48;i++) {
            //usleep(8000); //TODO hopefully get to remove
            tx[0] = 0x6D;
            tx[1] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
            tx[2] = (uint8_t)g[i].player;
            tx[3] = (uint8_t)g[i].occupy_type;
            tx[4] = i; //country code
            tx[5] = 0x00;
            tx[6] = 0x00;
            printf("Sent region %i: player - %i, type - %i\r\n",i,tx[2],tx[3]);
            //printf("hit enter to continue ");
            //x = getchar();
            txd(0, tx);
            //txdata(4, tx);
        } 

        printf("hit enter to continue to lock sig");
        int x = getchar();
        x = getchar();

        //3. Wait for turn_up || btn_press --> send 0xBF ack
        tx[0] = 0x6D;
        tx[1] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
        tx[2] = 0x3F; //round end signal
        tx[3] = 0x00;
        tx[4] = 0x00; //N/A
        tx[5] = 0x00;
        tx[6] = 0x00;
        //txdata(4, tx);
        txd(0, tx);

        printf("Starting polling!\r\n");
        //4. Start Polling to get orders from controllers
        for (i = 0;i < 1;i++) {
        //for (i = 0;i < 3;i++) {
            tx[0] = 0x6D;
            tx[1] = ackcount[i]; //splits to send seperate acknowledge count
            tx[2] = (uint8_t) i; //player id
            tx[3] = 0x00;
            tx[4] = 0x00; 
            tx[5] = 0x00;
            tx[6] = 0x00;
            txd(5, tx); //get order data back

        //8. Send order data to arbitrator
            numO = numinc;
            arbitor();


        }

        //clear acknowledge counts
        for (i = 0;i < 4;i++) ackcount[i] = 0;
        printf("End of Transmission!!!\r\n");
        clean();
        //printf("Done with arbitor - hit enter to continue!\r\n");
        //x = getchar();
   }
}

