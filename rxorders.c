#include <stdio.h>
#include <stdlib.h>
#include "region.h"
#include "order.h" 
#include "include.h"

/*
 * Holds functions for uart and wireless communication
 * *writes valid orders to /tmp/outfile TODO for now
 *
 * -------------------------------------------------
 *  Basic polling communication
 *  During waits send check byte to each controller (keyed for wireless com)
 *  if receieves a positive signal then transfers orders in
 *  after its recieved orders it runs against firstvalidate()
 *  returning results of invalid orders to the player
 *  then takes valid orders and checks against older conflicting orders from 
 *  same player 
 *  replacing old order with newer ones we then save off the confirmed results
 *  in a temporary orders file associated with the player
 *  Once the round is complete all the players order files is transferred into
 *  the main orders file to be analysed by the arbitrator
 */

//TODO check pins for uart com and port name
int uartsetup() {
/*
    int uart0_fs = -1;
    // read write, prevents device becoming controller for process, non blocking
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
*/
}
//called during game wait periods checking for submitted orders
int checkfororders(){
    uartsetup();
    /*
     *
    //----- TX BYTES -----
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    
    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = 'H';
    *p_tx_buffer++ = 'e';
    *p_tx_buffer++ = 'l';
    *p_tx_buffer++ = 'l';
    *p_tx_buffer++ = 'o';
    
    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));      //Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART TX error\n");
        }
    }
//----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);      //Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0)
        {
            //An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0)
        {
            //No data waiting
        }
        else
        {
            //Bytes received
            rx_buffer[rx_length] = '\0';
            printf("%i bytes read : %s\n", rx_length, rx_buffer);
        }
    }

    //close uart 
    close(uart0_fs);
     */

}
