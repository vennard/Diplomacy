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
#define TIMEOUT 5000  //in clock cycles 
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
static uint32_t speed = 1000000;
static uint16_t delay = 10;

static void pabort(const char *s) {
  perror(s);
  abort();
}

void changemode(int mode);
uint8_t send(int len, uint8_t *in);  
void setupspi();
void rxdata(int len); 

//Address, Data
//38 elements
uint8_t config[38][2] = { 
    { _FSCTRL1, 0x08 },
    { _FSCTRL0, 0x00 },
    { _FREQ2, 0x23   },
    { _FREQ1, 0x31   },
    { _FREQ0, 0x3B   },
    { _MDMCFG4, 0xCA },
    { _MDMCFG3, 0x83 },
    { _MDMCFG2, 0x93 },
    { _MDMCFG1, 0x22 },
    { _MDMCFG0, 0xF8 },
    { _CHANNR, 0x00  },
    { _DEVIATN, 0x34 },
    { _FREND1, 0x56  },
    { _FREND0, 0x10  },
    { _MCSM0, 0x18   },
    { _FOCCFG, 0x16  },
    { _BSCFG, 0x6C   },  
    { _AGCCTRL2, 0x43 },
    { _AGCCTRL1, 0x40 },
    { _FSCAL3, 0x91  }, 
    { _FSCAL2, 0x29  }, 
    { _FSCAL1, 0x2A  }, 
    { _FSCAL0, 0x00  }, 
    { _FSCAL0, 0x1F  }, 
    { _FSTEST, 0x59  }, 
    { _TEST2, 0x81   },
    { _TEST1, 0x35   },
    { _TEST0, 0x09   },
    { _FIFOTHR, 0x47 },
    { _IOCFG2, 0x29  },
    { _IOCFG0, 0x06  },
    { _PKTCTRL1, 0x08},
    { _PKTCTRL0, 0x04},
    { _ADDR, 0x00    },
    { _PKTLEN, 0x07  },
    { _WORCTRL, 0xFB },
    { _SYNC1, 0xDD   },
    { _SYNC0, 0xDD   },
};

uint8_t readingdata[64];
unsigned char ackcount[4];
order_t inputorders[256][4];
int fd;
int ocount = 0;

void testfunc() {
printf("CALLED TESTING FUNCTION");
}

void flushbuffer() {
	//printf("Flushing RX Buffer Manually...");
	uint8_t tx[60];
	tx[0] = RX_BURST;
	changemode(2);
	send(60,tx);
	//printf(" finished flushing!!!! please work\r\n");
}


//waits for acknowledge(s) from controller(s)
//type 0 - 3 = controller to wait for
//type 4 = wait for ack from all controllers
//return - 0 if successfully got acknowledge(s)
//         1 - timed out! retry comm and call again
int ackwait(int type, int messageid) {
    //time_t tstart = time(NULL);
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
		//printf("\r\nLOOKING FOR ACK\r\n");
                rxdata(7); //read 2 byte acknowledge
		//printf("DONE LOOKING FOR ACK\r\n\r\n");
	    
		//printf("\r\nLOOKING FOR ACK\r\n");
                //rxdata(7); //read 2 byte acknowledge
		//printf("DONE LOOKING FOR ACK\r\n\r\n");
		//testfunc();
                if ((readingdata[2] == 0xAC)&&(readingdata[1] == type)&&(readingdata[3] == messageid)) {
                    //printf("Found controller %i acknowledge!\r\n",type);
		    changemode(0);
		    usleep(2500);
		    uint8_t clearbuf = _SFRX;
		    send(1, &clearbuf);
		    //flushbuffer();
		    usleep(2500);
		    //printf("Cleared RX buffer!\r\n");
                    count++;
                } else {
                    //int telapsed = difftime(time(NULL), tstart);
		  //  usleep(2500);
		//	return 1;
		    clock_t now = clock();
		    int telapsed = now - tstart;
                    //printf("ERROR: found invalid ack! TIME ELAPSED - %i (now %i - tstart %i)\r\n",telapsed,(int)now,(int)tstart);
		    if ((telapsed < 0)||(telapsed > TIMEOUT)){
                        //printf("COM ERROR: Timed out after %i seconds waiting for acks!\r\n",TIMEOUT);
                        return 1; //TODO need this
                    }
                }
            }
            ackcount[type]++;//Increment acknowledge count
            break;
        case 4:
            //printf("Waiting for acknowledge from all 4 controllers...\r\n");
            while (count < 4) {
                rxdata(2); //read 2 byte acknowledge
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
            //Increment acknowledge count
            for (i = 0;i < 4;i++) ackcount[i]++;
            break;
        default:
            printf("Invalid ackwait call!\r\n");
            return 1;
            break;
    }
    return 0;
}
//Simple read of len bytes off of the RX fifo
//writes to readingdata[]
void rxdata(int len) {
    int i, ret;
    uint8_t tx[len+1]; 
    //check for correct number of bytes
	int done = 0;
	while (done == 0) {
    		changemode(0);
    		tx[0] = _PKTSTATUS | 0x80;
    		tx[1] = 0x00;
		uint8_t pck = send(2, tx);
		uint8_t gdo2 = pck & 0x04;
    		//changemode(2); //put in rx 
		tx[0] = _RXBYTES | 0x80;
    		uint8_t result = send(2, tx);
		result = result & 0x7f;
		tx[0] = _SNOP;
		tx[1] = _SNOP;
		uint8_t nopresult = send(2,tx);
		if ((gdo2 == 0x04)&&(result > 6)) {
    			changemode(2); //put in rx 
			//printf("CRC CHECK OK!\r\n");
    			//printf("Reading %i bytes...                    region ------------------------------------> %i ",len,ocount);
    			if (len < 1) printf("Receive request invalid - requested 0 bytes!\r\n");
    			if (len < 2) {
        			tx[0] = RX_BYTE;
    			} else {
        			tx[0] = RX_BURST;
    			}
    			struct spi_ioc_transfer fr = {
        			.tx_buf = (unsigned long) tx,
        			.rx_buf = (unsigned long) readingdata,
        			.len = len+1, //TODO changed from +1
        			.delay_usecs = delay,
        			.speed_hz = speed,
        			.bits_per_word = bits,
    			};
    			ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
    			if (ret < 1) pabort("can't send spi message");
    			//printf("\r\nRX: ");
    			//for (i = 0;i < len+1;i++) printf("0x%x ",readingdata[i]);
    			//for (i = 0;i < len+1;i++) printf("0x%x ",readingdata[i]);
    			//printf("\r\n");

    //check status byte
    			uint8_t check = readingdata[0] & 0x70;
    			if (check != 0x10) printf("WARNING: READ STATUS RETURN GAVE NON-RX VALUE: 0x%x!!!!\r\n",readingdata[0]);
			done = 1;
		} 
	}
}

//transfers data via radio
//default is 7 byte transfer burst (Not including header for spi)
//acktype is needed acknowledge address (4 for all)
//returns 0 on success, 1 otherwise
void txdata(int acktype, uint8_t *in) {
    int i, ret;
    int ackd = 0;
    uint8_t tx[8]; 
    uint8_t rx[8]; 
    for (i = 1;i < 8;i++) tx[i] = in[i];
    while (ackd == 0) {
    	tx[0] = TX_BURST;
        changemode(1);
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
	//printf("\r\n\r\nSent Data: ");
	//for (i = 0;i < 8;i++) printf("0x%x ",tx[i]);
	//printf("done!\r\n\r\n");
	//TODO END OF KNOWN GOOD TODO
        //check that status bit = tx mode
	usleep(2500);	//ABSOLUTELY NECESSARY DO NOT REMOVE!!!!!!!!!!!!!!!!!!!!!!!
	//char buf[1];
	//scanf("%s",buf);
        uint8_t check = rx[0] & 0x70;
        if (check != 0x20) {
            printf("ERROR: CC1101 not in tx mode!\r\n");
            changemode(0); //send back to idle
        } else if (ackwait(acktype,tx[2]) == 1) {
            //printf("ERROR: Failed to get acknowledges!\r\n");
        } else {
            //printf("SUCCESS! Data: ");
            for (i = 1;i < 8;i++) printf("0x%x ",tx[i]);
            //printf(" sent and acknowledged.\r\n");
            ackd = 1;
        }
    }
}

//Sends data over spi
//len = number of bytes to be sent via spi
//returns last rx byte received from cc1101
uint8_t send(int len, uint8_t *in) { 
    int i, ret;
    uint8_t rx[len];
    if (len >= 1){
        //printf("Sending %i bytes: ",len);
        //for (i = 0;i < len;i++) printf("0x%x ",in[i]);
        //printf("---------");
    } else {
        printf("Send invalid - requested 0 bytes!\r\n");
    }
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
    return rx[len-1];  
} 

//Write all data necessary to configure the CC1101
void configurespi() {
    printf("Setting up CC1101...\r\n");
    fd = open(device, O_RDWR);
    if (fd < 0) pabort("can't open device");
    setupspi();
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    uint8_t tx[2];
    uint8_t check;
    tx[0] = 0x30; 
    send(1, &tx[0]);  //reset chip 
    printf("reset!\r\n");
    changemode(0); //put in idle
    printf("Configuring... \r\n");
    int i;
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
        printf("    ... got 0x%x",check);
        if (check != config[i][1]) {
            printf(" validation FAILED! retrying...\r\n"); 
            i--;
        } else {
            printf(" success!\r\n");
        }
        //sleep(1);
    }
    changemode(0); //put in idle
    tx[0] = _SFTX;
    send(1,tx);
    printf("Cleared TX Buffer... ");
    tx[0] = _SFRX;
    send(1,tx);
    printf("Cleared RX Buffer!\r\n");
    printf("------------------Completed CC1101 configuration successfully!-----------------------\r\n\r\n");
}

void setupspi() {
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
void tx_phase_start(int phasetype, void* ptr) {
    //uint8_t tx[64];
    //uint8_t rx[64];
    //region_t *g = (region_t*)ptr;
    ptr = NULL;
    switch(phasetype) {
        case 1:
            printf("Starting retreat phase rf/spi communication\r\n");
            break;
        case 2:
            printf("Starting supply phase rf/spi communication\r\n");
            break;
        default:
            printf("Starting normal orders phase rf/spi communication\r\n");
            break;
    }
    //TODO copy from below 
}

//starts after timer completes || btn_press
//Gets all order data from controllers
//roundtype - 0 normal, 1 retreat/disband, 2 supply
//saves orders in inputorders[][] TODO will have to put in global o[]
//returns number of new orders
int rx_orders_start(int roundtype) {
    void *ptr;
    //TODO copy from below
    return 0;
}

void demo(void) {
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
    configurespi();

    //2. Send Phase Type (Key & Data) 2 bytes
    printf("2. Sending phase type:\r\n");
    tx[0] = TX_BURST;
    tx[1] = 0xED; //sender addr
    tx[2] = ackcount[0]; 
    tx[3] = 0x88; //phase type key
    tx[4] = 0x00; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
    tx[5] = 0x00; //N/A
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(0, tx);

    //3. Send out country data
    printf("3. Sending phase type:\r\n");
    tx[0] = TX_BURST;
    tx[1] = 0xED; //sender addr
    tx[2] = ackcount[0]; 
    tx[3] = (uint8_t) g[country].player;
    tx[4] = (uint8_t) g[country].occupy_type;
    tx[5] = (uint8_t) country; 
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(0, tx);

    //4. wait 1 minute turn period
    printf("Starting wait period of 1 min...");
    settmr(1);
    starttmr();
    while (checktmr() == 0); //spin on timer status
    printf("Timer done!\r\n");

    //5. Send lock to controller
    printf("Sending lock signal to controller!\r\n");
    tx[1] = 0xED;
    tx[2] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
    tx[3] = 0xBF; //round end signal
    tx[4] = 0x00;
    tx[5] = 0x00; //N/A
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(0, tx);

    //6. Zedboard polls controller
    printf("Sending polling signal to controller!\r\n");
    tx[1] = 0xED;
    tx[2] = ackcount[0]; //splits to send seperate acknowledge count
    tx[3] = 0; //player id
    tx[4] = 0x00;
    tx[5] = 0x00; 
    tx[6] = 0x00;
    tx[7] = 0x00;
    txdata(0, tx);

    //7. Controller sends order data back
    printf("waiting for controller order data... ");
    int done = 0;
    order_t no;
    while (!done) {

        char buf[1];
        printf("enter to continue");
        scanf("%s",buf);

        rxdata(7); //puts results in readingdata 1 -> len
        printf("Status of RX: 0x%x ",readingdata[0]);
        //check if valid
        if ((readingdata[1] == 0)&&(readingdata[2] == ackcount[0])) {
            printf(" found valid order!\r\n");
            no.player = 0;
            no.country = readingdata[3]; 
            no.order = readingdata[4]; 
            no.type = readingdata[5]; 
            no.tcountry = readingdata[6]; 
            no.scountry = readingdata[7]; 
            printf("Order was: player %x, country %x, order %x, unit type %x, type %x, tcountry %x, scountry %x!\r\n",no.player,no.country,no.order,no.type,no.tcountry,no.scountry);
            done = 1;
            printf("Sending ack!\r\n");
            tx[0] = TX_BURST;
            tx[1] = 0xED;
            tx[2] = 0xAC;
            tx[3] = ackcount[0];
            txdata(0, tx);
        } else if (readingdata[1] == 0) {
            printf(" found valid player id, not accepting but still sending ack\r\n");
            printf("Sending ack!\r\n");
            tx[0] = TX_BURST;
            tx[1] = 0xED;
            tx[2] = 0xAC;
            tx[3] = ackcount[0];
            txdata(0, tx);
        } else {
            printf(" found invalid order -- ERROR!\r\n");
        }
    }

    //8. Send order data to arbitrator
    o[0] = no; //save off order
    numO = 1;
    arbitor();
}

void runspi(void) {
    printf("Called runspi!\r\n"); 
    configurespi();

    //Transmit test game data
    uint8_t tx[64];
    uint8_t rx[64];
    while(1) {
        changemode(0); //put in idle
	tx[0] = _SFRX;
	send(1,tx);
        printf("\r\nStart of Transmission!!!\r\n");

        //1. Send Phase Type (Key & Data) 2 bytes
        printf("1. Sending phase type:\r\n");
        tx[0] = TX_BURST;
        tx[1] = 0xED; //sender addr
        tx[2] = ackcount[0]; 
        tx[3] = 0x88; //phase type key
        tx[4] = 0x00; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
        tx[5] = 0x00; //N/A
        tx[6] = 0x00;
        tx[7] = 0x00;
        //txdata(4, tx); //TODO needs this
        changemode(1); //put in tx mode                                                                
        txdata(0, tx);
	
	char buf[1];
	scanf("%s",buf);

        //2. Send region data 0 -> 47 (Owner & Unit_type) 2 bytes
        int i;
        printf("2. Sending region data:\r\n");
        for (i = 0;i < 48;i++) {
            tx[1] = 0xED;
            tx[2] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
            tx[3] = (uint8_t)g[i].player;
            tx[4] = (uint8_t)g[i].occupy_type;
            tx[5] = 0x00; //N/A
            tx[6] = 0x00;
            tx[7] = 0x00;
            printf("%i: player - %i, type - %i\r\n\r\n",i,tx[3],tx[4]);
            changemode(1); //put in tx mode                                                                
            txdata(0, tx);
            //txdata(4, tx);
            usleep(2500); //TODO hopefully get to remove
	    printf("Sent Region %i!\r\n",i);
	    ocount++;
        } 

	scanf("%s",buf);

        //3. Wait for turn_up || btn_press --> send 0xBF ack
        sleep(10); //TODO need to replace with game state wait
        tx[1] = 0xED;
        tx[2] = ackcount[0]; //TODO using controller 0 as master -- maybe take mean here or diff?
        tx[3] = 0xBF; //round end signal
        tx[4] = 0x00;
        tx[5] = 0x00; //N/A
        tx[6] = 0x00;
        tx[7] = 0x00;
        changemode(1); //put in tx mode                                                                
        txdata(4, tx);

        //4. Start Polling to get orders from controllers
        for (i = 0;i < 3;i++) {
            //Flush RX Buffer before requesting transfer
            changemode(0);
            tx[0] = TX_BYTE;
            tx[1] = _SFTX;
            send(2, tx); //UM WHAAAAT TODO fix this shit

            //Send out player ID
            tx[1] = 0xED;
            tx[2] = ackcount[i]; //splits to send seperate acknowledge count
            tx[3] = (uint8_t) i; //player id
            tx[4] = 0x00;
            tx[5] = 0x00; 
            tx[6] = 0x00;
            tx[7] = 0x00;
            changemode(1); //put in tx mode                                
            txdata(i, tx);

            //get orders until seen 0xDE (done char) 
            changemode(0);
            uint8_t done = 0x00;
            printf("Starting wait loop for controller %i order data:",i);
            int count = 0;
            while (done != 0xDE) {
                printf("Waiting for packet... ");
                //int readcheck = rxd(); //TODO add back
		int readcheck = 1; //TODO remove
                if (readcheck == 0) { //valid read 
                    //check for done byte
                    if (readingdata[3] == 0xDE) { //found done byte
                        printf("Seen done byte 0xDE!!\r\n");
                        done = 0xDE;
                    } else {
                        //save off order data
                        printf("Saving order #%i!\r\n",count);
                        inputorders[count][i].country = readingdata[3];
                        inputorders[count][i].order = readingdata[4];
                        inputorders[count][i].type = readingdata[5];
                        inputorders[count][i].tcountry = readingdata[6];
                        inputorders[count][i].scountry = readingdata[7];
                        count++;
                    }
                } else {
                    printf("Got invalid rxd response, ignore read and retrying\r\n");
                }
            }
            sleep(1);
        }

        //clear acknowledge counts
        for (i = 0;i < 4;i++) ackcount[i] = 0;
        printf("Done getting order data... launching arbirator\r\n");
        //TODO add call to arbitrator passing it inputorders[][]
        sleep(5);
        printf("End of Transmission!!!\r\n");
    }
}
     
