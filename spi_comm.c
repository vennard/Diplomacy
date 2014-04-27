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
#include "region.h"
#include "order.h"
#include "include.h"

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
///////////////////////////////////////////////////////////////////////////////////////
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
//static uint32_t speed = 500000;
static uint32_t speed = 1000000;
//static uint32_t speed = 250000;
static uint16_t delay = 10;
static void pabort(const char *s) {
  perror(s);
  abort();
}

void changemode(int fd, int mode);

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
    { _FSCAL2, 0xE9  }, 
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
    { _PKTCTRL1, 0x04},
    { _PKTCTRL0, 0x04},
    { _ADDR, 0x00    },
    { _PKTLEN, 0x05  },
    { _WORCTRL, 0xFB },
    { _SYNC1, 0xCD   },
    { _SYNC0, 0xAB   },
};

//Reads len bytes off of the RX fifo
//passes back array with data starting at index = 1
uint8_t readingdata[64];
void rxdata(int fd, int len) {
    changemode(fd,0); //put in idle
    printf("Called rxdata, reading %i bytes off fifo... \r\n",len);
    int i, ret;
    uint8_t tx[len+1]; 
    if (len < 1) printf("Receive request invalid - requested 0 bytes!\r\n");
    if (len < 2) {
        tx[0] = TX_BYTE;
    } else {
        tx[0] = TX_BURST;
    }
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
    printf("RX Data: ");
    for (i = 1;i < len;i++) printf("0x%x ",readingdata[i]);
    printf("\r\n");
}

//Sends data over spi
//len = number of bytes to be sent via spi
//returns last rx byte received from cc1101
uint8_t send(int fd, int len, uint8_t *in) { 
    int i, ret;
    uint8_t tx[len]; 
    uint8_t rx[len];
    if (len < 1) printf("Send invalid - requested 0 bytes!\r\n");
    for (i = 0;i < len;i++) { 
        tx[i] = in[i];
    }
    if (len >= 1){
        printf("Sending %i bytes: ",len);
        for (i = 0;i < len;i++) printf("0x%x ",tx[i+1]);
        printf("---------");
    } 
    struct spi_ioc_transfer fr = {
        .tx_buf = (unsigned long) tx,
        .rx_buf = (unsigned long) rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
    if (ret < 1) pabort("can't send spi message");
    if (len >= 1) {
        printf("RX: ");
        for(i = 0;i < len;i++) printf("0x%x ",rx[i]);
        printf("\r\n");
    }
    return rx[len-1];  
} 

static void configure(int fd) {
    printf("Configuring CC1101... ");
    uint8_t tx[2];
    uint8_t check;
    tx[0] = 0x30;
    send(fd, 1, &tx[0]);  //reset chip 
    sleep(2);
    printf("reset!\r\n");
    changemode(fd, 0); //put in idle
    sleep(1);
    printf("Configuring... ");
    int i;
    for (i = 0;i < 38;i++) {
        printf("%i ",i);
        tx[0] = config[i][0]; 
        tx[1] = config[i][1];
        send(fd, 2, &tx[0]);
        sleep(1);
        check = config[i][0] | 0x80;
        tx[0] = check;
        tx[1] = 0x00;
        check = send(fd, 2, &tx[0]);
        printf("checking wrote 0x%x to reg 0x%x, read back 0x%x... ",config[i][1],config[i][0],check);
        if (check != config[i][1]) {
            printf(" validation FAILED! retrying...\r\n"); 
            i--;
            sleep(1);
        } else {
            printf(" success!\r\n");
        }
    }
}

void setupspi(int fd) {
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
void changemode(int fd, int mode) {
    //send nop to check status
    uint8_t state, check;
    uint8_t out;
    check == -1;
    switch (mode) {
        case 1: 
            state = CHIP_STATE_TX;
            break;
        case 2: 
            state = CHIP_STATE_RX;
            break;
        default:
            state = CHIP_STATE_IDLE;
            break;
    }
    out = _SNOP; 
    check = send(fd, 1, &out);
    check = check & 0x70;
    while(state != check) {
        switch (check) {
            case CHIP_STATE_SETTLING:
                printf("Chip is settling... waiting\r\n");
                break;
            case CHIP_STATE_CALIBRATE:
                printf("Chip is calibrating... waiting\r\n");
                break;
            case CHIP_STATE_TXFIFO_UNDERFLOW:
                printf("TX underflow detected, sending SFTX\r\n");
                out = _SFTX;
                send(fd, 1, &out);
                break;
            case CHIP_STATE_RXFIFO_OVERFLOW:
                printf("RX overflow detected, sending SFRX\r\n");
                out = _SFRX;
                send(fd, 1, &out);
                break;
            case CHIP_STATE_FSTON: 
                printf("Fast TX ready state detected, sending SIDLE\r\n");
                out = _SIDLE;
                send(fd, 1, &out);
                break;
            case CHIP_STATE_RX: 
                printf("RX state detected, sending SIDLE\r\n");
                out = _SIDLE;
                send(fd, 1, &out);
                break;
            case CHIP_STATE_TX: 
                printf("TX state detected, sending SIDLE\r\n");
                out = _SIDLE;
                send(fd, 1, &out);
            case CHIP_STATE_IDLE:
                printf("IDLE state detected, ");
                if (state == CHIP_STATE_TX) {
                    printf("sending to tx state\r\n");      
                    out = _STX;
                    send(fd, 1, &out);
                } else if (state == CHIP_STATE_RX) {
                    printf("sending to rx state\r\n");      
                    out = _SRX;
                    send(fd, 1, &out);
                } else {
                    printf("continuing\r\n");      
                }
                break;
            default:
                break;
        }
        sleep(1);
        printf("Rechecking... ");
        out = _SNOP; 
        check = send(fd, 1, &out);
        check = check & 0x70;
    }
    printf("found correct state! continuing...\r\n");
}

int runspi(void *g) {
    printf("Called runspi!\r\n"); 
    region_t *gd = (region_t*)g;
    int fd = open(device, O_RDWR);
    if (fd < 0) pabort("can't open device");
    setupspi(fd);
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    configure(fd);

    //Transmit test game data
    uint8_t tx[64];
    uint8_t rx[64];
    while(1) {
        printf("\r\nStart of Transmission!!!\r\n");
        changemode(fd,1); //put in tx mode                                                                

        //1. Send Phase Type (Key & Data) 2 bytes
        tx[0] = TX_BURST;
        tx[1] = 0x88; //key for phase  
        tx[2] = 0x00; //Phase type, 0 - order writing, 1 - locked, 2 - retreat/disband, 3 - gain/lose units
        send(fd, 3, tx);

        //2. Send region data 0 -> 47 (Owner & Unit_type) 2 bytes
        int i;
        printf("Sending region data...\r\n");
        for (i = 0;i < 48;i++) {
            tx[1] = (uint8_t)gd[i].player;
            tx[2] = (uint8_t)gd[i].occupy_type;
            printf("%i: player - %i, type - %i\r\n",i,tx[1],tx[2]);
            changemode(fd,1); //put in tx mode                                                                
            send(fd, 3, tx);
            sleep(1); //TODO hopefully get to remove
        } 

        //3. Wait for turn_up || btn_press --> send 0xBF ack
        sleep(10); //TODO need to replace with game state wait
        tx[0] = TX_BYTE;
        tx[1] = 0xBF;
        changemode(fd,1); //put in tx mode                                                                
        send(fd, 2, tx);

        //4. Start Polling to get orders from controllers
        for (i = 0;i < 3;i++) {
            //Flush RX Buffer before requesting transfer
            changemode(fd,0);
            //Send out player ID
            tx[0] = TX_BYTE; 
            tx[1] = (uint8_t)i;
            changemode(fd,1); //put in tx mode                                                                
            send(fd, 2, tx);

            //get orders until seen 0xDE (done char) 
            tx[0] = _SFRX; 
            changemode(fd,0);
            uint8_t done = 0x00;
            printf("Starting wait loop for controller order data:");
            while (done != 0xDE) {
                printf("Waiting for packet... ");
                rxdata(fd, 5); //read 5 bytes from fifo: ordertype, unittype, c, tc, sc
                //check for complete byte
                if (readingdata[1] == 0xDE) {
                    printf("Got complete byte 0xDE!! Continuing\r\n");
                    done = 0xDE;
                } else {
                    //TODO save off read data
                    printf("Saving off Data -- TODO\r\n");
                }
            }
            sleep(1);
        }
        printf("Done getting order data... launching arbirator\r\n");
        sleep(5);
        printf("End of Transmission!!!\r\n");
    }
    return 0;
}
     
