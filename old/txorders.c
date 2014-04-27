#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "order.h"
#include "region.h"
#include "include.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MCP23S08_IODIR 		0x00 
#define MCP23S08_IPOL 		0x01 
#define MCP23S08_GPINTEN 	0x02 
#define MCP23S08_DEFVAL 	0x03 
#define MCP23S08_INTCON 	0x04 
#define MCP23S08_IOCON 		0x05 
#define MCP23S08_GPPU 		0x06 
#define MCP23S08_INTF 		0x07 
#define MCP23S08_INTCAP 	0x08 
#define MCP23S08_GPIO 		0x09 
#define MCP23S08_OLAT 		0x0A

#define MCP23S08_IOCON_HAEN (1 << 3)

#define MCP23S08_ADDR		0x40

static void pabort(const char *s) {
    perror(s);
    abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

uint8_t mcp_iocon[]     = { MCP23S08_ADDR, MCP23S08_IOCON,MCP23S08_IOCON_HAEN};
uint8_t mcp_gpp[]       = { MCP23S08_ADDR, MCP23S08_GPPU,0x00};
uint8_t mcp_ipol[]      = { MCP23S08_ADDR, MCP23S08_IPOL,0x00};
uint8_t mcp_inten[]     = { MCP23S08_ADDR, MCP23S08_GPINTEN,0x00};
uint8_t mcp_iodir[]     = { MCP23S08_ADDR, MCP23S08_IODIR,0x00};
uint8_t mcp_gpio[]      = { MCP23S08_ADDR, MCP23S08_GPIO,0x00};

static void transfer(int fd, uint8_t *tx)
{
        int ret;
        uint8_t rx[3] = {0,0,0};
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx,
                .rx_buf = (unsigned long)rx,
                .len = 3,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1) pabort("can't send spi message");

        for (ret = 0; ret < 3; ret++) {
            if (!(ret % 6)) puts("");
            printf("%.2X ", rx[ret]);
        }
        puts("");
}

int main(int argc, char *argv[]) {
    int ret = 0;
    uint8_t digit = 0x01;
    int fd;

    //would parse opts here

    fd = open(device, O_RDWR);
    if (fd < 0) pabort("can't open device");

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) pabort("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) pabort("can't get max speed hz");

    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

    uint8_t temp = 15;
    /*
    transfer(fd, mcp_iocon);
    transfer(fd, mcp_gpp);
    transfer(fd, mcp_ipol);
    transfer(fd, mcp_inten);
    transfer(fd, mcp_iodir);
    */

    while(1) {
        transfer(fd, temp);
        sleep(5);
    }
    close(fd);
    return ret;
}
//TODO end of added code
/*
int num_orders;
int main(int argc, char *argv[]) {
    //int num_o = getTestOrders(1,num_orders,"/tmp/torders"); //order in o[0] global var
    //printf("Generating %i orders to be transferred\r\n",num_o);
    //Setup UART
    int uart0_fs = -1;
    // read write, prevents device becoming controller for process, non blocking
    // TODO rename port
    uart0_fs = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_fs == -1) {
        perror("cant open serial port");
        return -1;
    }
    struct termios options;
    tcgetattr(uart0_fs, &options);
    // baud, size, ignore modem status lines, enable reciever
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR; //ignore parity errors
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_fs, TCIFLUSH);
    tcsetattr(uart0_fs, TCSANOW, &options);

    unsigned char txbuff[20];
    unsigned char *ptx;

    ptx = &txbuff[0];
    *ptx++ = 't';
    *ptx++ = 'e';
    *ptx++ = 's';
    *ptx++ = 't';



    if (uart0_fs != -1) {
        int count = write(uart0_fs, &txbuff[0], (ptx - &txbuff[0]));
        if (count < 0) {
            printf("UART TX error\r\n");
        }
    }
    
}
*/
