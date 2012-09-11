#include <termios.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include "settings.h" 


/** See http://en.wikibooks.org/wiki/Serial_Programming/Serial_Linux for more information */
struct termios tio;
int tty_fd;
unsigned char buf;
int connect_to_serial() {
memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
 
        tty_fd=open(SERIALPORT, O_RDWR | O_NONBLOCK);   
	printf("tty_fd %d\n",tty_fd);   
        cfsetospeed(&tio,B9600);            // 9600
        cfsetispeed(&tio,B9600);            // 9600
 
        tcsetattr(tty_fd,TCSANOW,&tio);
}

void has_valid_card() {
	unsigned char vbyte = 'v';
	int written = write(tty_fd,&vbyte,1);
}
void has_invalid_card() {
	char ibyte = 'i';
	write(tty_fd,&ibyte,1);
}
