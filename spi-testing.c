/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
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


///////////////////////////////////////////////////////////////////////////////////////
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


static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
//static uint32_t speed = 50000;
static uint32_t speed = 500000;
//static uint32_t speed = 250000;
static uint16_t delay = 10;

uint8_t testdata[] = { 0xAA, 0xFF, 0x00 };

uint8_t rf2addresses[] = {
	0x0B,  //FSCTRL1
	0x0C,  //FSCTRL0
	0x0D,  //FREQ2
	0x0E,  //FREQ1
	0x0F,  //FREQ0
	0x10,  //MDMCFG4
	0x11,  //MDMCFG3
	0x12,  //MDMCFG2
	0x13,  //MDMCFG1
	0x14,  //MDMCFG0
	0x0A,  //CHANNR
	0x15,  //DEVIATN
	0x21,  //FREND1
	0x22,  //FREND0
	0x18,  //MCSM0
	0x19,  //FOCCFG
	0x1A,  //BSCFG
	0x1B,  //AGCTRL2
	0x1C,  //AGCTRL1
	0x1D,  //AGCTRL0
	0x23,  //FSCAL3
	0x24,  //FSCAL2
	0x25,  //FSCAL1
	0x26,  //FSCAL0
	0x29,  //FSTEST
	0x2C,  //TEST2
	0x2D,  //TEST1
	0x2E,  //TEST0
	0x03,  //FIFOTHR
	0x00,  //IOCFG2
	0x02,  //IOCFG0
	0x07,  //PKTCTRL1
	0x08,  //PKTCTRL0
	0x09,  //ADDR
	0x06,  //PKTLEN
	0x20,  //WORCTRL
	0x04,  //SYNC1
	0x05,  //SYNC0
};

uint8_t rf2Settings[] = {
	0x08,  //Data for FSCTRL1 
	0x00,  //Data for FSCTRL0
	0x23,  //Data for FREQ2
	0x31,  //Data for FREQ1
	0x3B,  //Data for FREQ0
	0xCA,  //Data for MDMCFG4
	0x83,  //Data for MDMCFG3
	0x93,  //Data for MDMCFG2
	0x22,  //Data for MDMCFG1
	0xF8,  //Data for MDMCFG0
	0x00,  //Data for CHANNR
	0x34,  //Data for DEVIATN
	0x56,  //Data for FREND1
	0x10,  //Data for FREND0
	0x18,  //Data for MCSM0
	0x16,  //Data for FOCCFG
	0x6C,  //Data for BSCFG
	0x43,  //Data for AGCTRL2
	0x40,  //Data for AGCTRL1
	0x91,  //Data for AGCTRL0
	0xE9,  //Data for FSCAL3
	0x2A,  //Data for FSCAL2
	0x00,  //Data for FSCAL1
	0x1F,  //Data for FSCAL0
	0x59,  //Data for FSTEST
	0x81,  //Data for TEST2
	0x35,  //Data for TEST1
	0x09,  //Data for TEST0
	0x47,  //Data for FIFOTHR
	0x29,  //Data for IOCFG2
	0x06,  //Data for IOCFG0
	0x04,  //Data for PKTCTRL1
	0x04,  //Data for PKTCTRL0
	0x00,  //Data for ADDR
	0x05,  //Data for PKTLEN
	0xFB,  //Data for WORCTRL
	0xAB,  //Data for SYNC1
	0xCD,  //Data for SYNC0
	
};

static void transfer2(int fd, uint8_t *header, uint8_t *tx)
{
	int ret;
	uint8_t rx[2] = {0,0};
	uint8_t ttx[2] = {*header, *tx};
	struct spi_ioc_transfer fr = {
		.tx_buf = (unsigned long)ttx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
	if (ret < 1) pabort("can't send spi message");
	printf("TRANSMIT COMPLETE: SENT - 0x%x 0x%x  RECEIVED - 0x%x 0x%x!\r\n",ttx[0],ttx[1],rx[0],rx[1]);
}

uint8_t checkstatus(int fd) {
	printf("--------------------------Checking status... ");
	int chiprdy = 0;
	int ret;
	uint8_t mask1;
	uint8_t rx[2] = {0,0};
	uint8_t tx[2] = {0x3D, 0x3D};
	while (chiprdy == 0) {
		struct spi_ioc_transfer fr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		};
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
		if (ret < 1) pabort("can't send spi message");
		uint8_t rdychk = rx[1] & 0x80;
		mask1 = rx[1] & 0x70;
		if (rdychk == 0x80) {
			printf("CHIP NOT READY! retrying...\r\n");
			chiprdy = 0;
		} else {
			chiprdy = 1;
			printf("\r\nComplete: sent - 0x%x 0x%x\r\n         received - 0x%x 0x%x!\r\n",tx[0],tx[1],rx[0],rx[1]);
		}
	}
	return mask1;
}

static void configure(int fd) {
	printf("Configuring CC1101...\r\n");
	int ret, i;
	uint8_t tx[2];
	uint8_t rx[2];
	uint8_t temp;
	int ready = 0;
	//Reset Chip and set to idle
	tx[0] = 0x30;
	struct spi_ioc_transfer fr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
	if (ret < 1) pabort("can't send spi message");
	printf("Reset CC1101 with 0x%x (response 0x%x)!\r\n",tx[0],rx[0]);
	sleep(5);

	//Wait until CC1101 is in idle before configuring
	printf("Chip has been reset... sending to idle state.\r\n ");
 	int idle = 0;	
	while (idle == 0) {
		sleep(1);	
		tx[0] = 0x36; //send SIDLE command
		tx[1] = 0x3D;
		transfer2(fd,&tx[0],&tx[1]);
		sleep(5);
		uint8_t stat = checkstatus(fd);
		printf("check returned - %x \r\n",stat);
		if (stat == 0x00) {
			idle = 1;
		
			printf("CC1101 is in IDLE. Continuing to configuration...\r\n");
		} else if (stat == 0x70) {
			sleep(1);
			printf("CC1101 is in TX_FIFO_UNDERFLOW. Sending SFTX signal...\r\n");
			tx[0] = 0x3B; //send SFTX command
			tx[1] = 0x3D;
			transfer2(fd,&tx[0],&tx[1]);
			sleep(1);
			//checkstatus(fd);
			printf("FINISHED TRYING TO FIX FIFO UNDERFLOW\r\n");
		} else {
			printf("CC101 is NOT IN IDLE... retrying\r\n");
		}
	}

	//Configure chip
	sleep(1);
	for (i = 0;i < 38;i++) {
		//write to configuration registers
		tx[0] = rf2addresses[i];	
		tx[1] = rf2Settings[i];
		printf("Command: 0x%x Data: 0x%x...",tx[0],tx[1]);
		//transfer2(fd,&tx[0],&tx[1]);
		struct spi_ioc_transfer rr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		};
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &rr);
		if (ret < 1) pabort("can't send spi message");

		uint8_t t3[2];
		t3[0] = rx[0];
		t3[1] = rx[1];
		printf("(status 0x%x 0x%x) ",rx[0],rx[1]);
		//sleep(1);
	 	//read from register and check expected val	
		temp = rf2addresses[i] | 0x80;
		tx[0] = temp;
		tx[1] = 0x00;
		struct spi_ioc_transfer gr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		};
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &gr);
		if (ret < 1) pabort("can't send spi message");
	//	sleep(1);
		printf("checking value (request 0x%x) - read 0x%x... ",temp,rx[1]);
		if (rx[1] == rf2Settings[i]) {
			printf("success!\r\n");
		} else {
			printf("failed!\r\n");
			i--;
			sleep(1);
		}
	}
	
	printf("DONE CONFIGURING CC1101!\r\n\r\n");
}
static void checkconfig(int fd) {
	printf("Check all configuration data...\r\n");
	int ret, i;
	uint8_t tx[2];
	uint8_t rx[2];
	uint8_t temp;
	for (i = 0;i < 36;i++) {
		temp = 0x80 | rf2addresses[i];	
		tx[0] = temp;
		tx[1] = 0x00;
		printf("Request = 0x%x ---------------",temp);
		struct spi_ioc_transfer fr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 2,
			.delay_usecs = delay,
			.speed_hz = speed,
			.bits_per_word = bits,
		};
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
		if (ret < 1) pabort("can't send spi message");
		printf("Addr: 0x%x - val:0x%x   exp:0x%x     ",rf2addresses[i],rx[1],rf2Settings[i]);
		if (rx[1] == rf2Settings[i]) {
			printf("success!\r\n");
		} else {
			printf("failed!\r\n");
		}
		sleep(1);
	}
}

void readfifo(int fd) 
{
	int ret;
	printf("Sending read command and nop...\r\n ");
	uint8_t tx[6] = {0x3D, 0x3D,0x3D,0x34,0x3D,0x3D};
	uint8_t rx[6] = {0,0,0,0,0,0};
	struct spi_ioc_transfer dr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 6,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &dr);
	if (ret < 1) pabort("can't send spi message");
	printf("Status: 0x%x!\r\n",rx[5]);
	uint8_t temp = rx[5] & 0xf0;
	if (temp == 0x70) {
		printf(" flushing tx buffer!\r\n");
		tx[0] = 0x3B;
		tx[1] = 0x34;
		transfer2(fd, &tx[0], &tx[1]);
	}
	sleep(1);
	printf("reading in 5 byte burst=");
	tx[0] = RX_BURST;
	tx[1] = 0x00;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 6,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) pabort("can't send spi message");
	int i;
 	for (i = 0;i < 6;i++) {
		printf("%i - 0x%x, ",i,rx[i]);
	}		
	printf("\r\n");
	tx[0] = 0x36;
	tx[1] = 0x3A;
	transfer2(fd, &tx[0], &tx[1]);
}

void transfer(int fd, uint8_t *header, uint8_t *tx)
{
	uint8_t txbuf[6] = {0x7F,0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
	uint8_t ttx[6] = {0x3B,0x35}; //flush tx buffer and enable tx
	uint8_t rx[6] = {0,0,0,0,0,0};
	int m;
	char n = 32;
	for (m = 1;m < 6;m++) {
		if(n == 0) n = 32;	
		txbuf[m] = n; 
		n++;
	}
	int ret;
	//uint8_t ttx[2] = {0x03,0x0C};
	/*
	struct spi_ioc_transfer dr = {
		.tx_buf = (unsigned long)ttx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &dr);
	if (ret < 1) pabort("can't send spi message");
*/
	printf("Transmitted: %x %x %x %x %x.\r\n",txbuf[1],txbuf[2],txbuf[3],txbuf[4],txbuf[5]);
	struct spi_ioc_transfer fr = {
		.tx_buf = (unsigned long)txbuf,
		.rx_buf = (unsigned long)rx,
		.len = 6,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
	if (ret < 1) pabort("can't send spi message");
	printf("Response: (status) 0x%x %x %x %x %x %x\r\n",rx[0],rx[1],rx[2],rx[3],rx[4],rx[5]);
//	sleep(2);
	/*
	ttx[0] = 0x3F;
	ttx[1] = 0xAA;
	
	struct spi_ioc_transfer dr = {
		.tx_buf = (unsigned long)ttx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &dr);
	if (ret < 1) pabort("can't send spi message");
	//TODO RX PROCESSING
	uint8_t rxbuf = rx[0] & 0x0F;
	printf("RX FIFO AT %x \r\n",rxbuf);
	if (rxbuf < 0x0F) {
		printf("RX FIFO has something in it!!!\r\n");
	}
	ttx[0] = 0x3F;
	ttx[1] = 0xFF;
	struct spi_ioc_transfer rr = {
		.tx_buf = (unsigned long)ttx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &rr);
	if (ret < 1) pabort("can't send spi message");

*/
}




static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	//uint8_t digit = 0x01;
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	uint8_t s1 = 0x30;
	uint8_t s2 = 0x3D; //nop -- check status bit
	//Transfer all configuration data to CC1101
	configure(fd); //TODO NEED THIS	
	//Check all configuration data
	//put in transmit mode
	s1 = 0x3B;
	s2 = 0x35;
	//Transmit actual data
	while(1)
	{
		printf("Start of Transmit... \r\n\r\n");
		transfer2(fd, &s1, &s2);
		sleep(2);
		printf(" cleared tx buffer... ");
		checkstatus(fd);
		uint8_t tx = TX_BURST;
		//uint8_t tx = TX_BYTE;
		uint8_t td = 0xAA;
		//uint8_t td = 0xAA;
		transfer(fd, &tx, &td);
		printf(" sent 5 byte burst.\r\n ");
		//readfifo(fd);
		sleep(1);
		tx = 0x36;
		td = 0x3D;
		transfer2(fd,&tx,&td);
		printf("...End of Transmit\r\n\r\n");
//TODO BELOW FOR TESTING
/*
	int ret;
	uint8_t rx[2] = {0,0};
	uint8_t tx[2] = {0x0d, 0x23};

	struct spi_ioc_transfer fr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &fr);
	if (ret < 1) pabort("can't send spi message");
	printf("Tried to write 0x23 - status: 0x%x, 0x%x... ",rx[0],rx[1]);
	//sleep(1);
	tx[0] = 0x8d; 
	tx[1] = 0x00;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) pabort("can't send spi message");
	printf("Read back: 0x%x, 0x%x\r\n",rx[0],rx[1]);
	sleep(1); 		
*/
#if 0
		digit = digit << 1;
		if( digit == 0)
		{
			digit = 0x01;
		}
		mcp_gpio[2] = ~digit;
#endif
	}
	close(fd);

	return ret;
}
