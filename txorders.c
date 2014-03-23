#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "order.h"
#include "region.h"
#include "include.h"

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

