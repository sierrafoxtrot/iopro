//
// capetool - cape io processor exercise tool.
// Copyright (C) 2013 Scott Finneran
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

//#include "/usr/include/linux/i2c.h"
#include "/usr/include/linux/i2c-dev.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Define device address
// It is a better idea to include all the addresses in a header file
#define DEVICE_ADDR 	0x04

using namespace std;

// Registers have the type in the upper nibble.
#define REG_TYPE(X) (((X) & 0xF0) >> 4)
#define REG_TYPE_ENCODE(X) (((X) & 0x0F) << 4)

// Registers have the index/number in the lower nibble.
#define REG_NUM(X) ((X) & 0x0F)

enum register_types
{
    REG_ID        = 0x00,
    REG_OUTPUT    = 0x01,
    REG_INVALID   = 0x0F,
};

int main(int argc, char *argv[])
{
    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msgs[2];
    int opt;
    uint8_t slaveAddress = DEVICE_ADDR;
    char data_read[8];
    char *busName = 0;
    uint8_t channelNumber = 0xFF;
    uint8_t newValue = 0xFF;

    while ((opt = getopt(argc, argv, "a:b:c:v:"))>0)
    {
        switch (opt) {
        case '?': puts("Bad argument"); break;
        case 'b': busName = (char *)malloc(strlen(optarg));
            strcpy(busName, optarg); break;
        case 'a': slaveAddress = atoi(optarg); break;
        case 'c': channelNumber = atoi(optarg); break;
        case 'v': newValue = atoi(optarg); break;
        default: break;
        }
    }

    int file = open(busName, O_RDWR);
    if (file < 0)
    {
        perror("i2cOpen in myI2C::i2cOpen\n");
        exit(1);
    }
    else
    {
        printf("Opened I/F\n");
    }

//    char data_to_write[2]={0x12,0};
//    char data_to_write[2]={0x12,1};
//    char data_to_write[2]={0x00,0};
    char data_to_write[2];

    // Query the module
    data_to_write[0] = REG_TYPE_ENCODE(REG_ID);

    msgs[0].addr  = slaveAddress;   //slave addr for write
    msgs[0].len   = 1;
    msgs[0].flags = 0;
    msgs[0].buf   = data_to_write;  /*subaddr ..*/

    msgs[1].addr  =  slaveAddress; //slave addr for read
    msgs[1].len   =  1;
    msgs[1].flags |= 0x01;         // read flag
    msgs[1].buf   =  data_read;

    msgset.nmsgs  = 2;
    msgset.msgs   = msgs;

    if (ioctl(file, I2C_RDWR, (unsigned long)&msgset) < 0)
    {
        printf("error I2C_RDWR errno=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    printf("Transaction complete: 0x%X 0x%X\n", data_read[0], data_read[1]);

    // Perform command
    data_to_write[0] = REG_TYPE_ENCODE(REG_OUTPUT);
    data_to_write[0] |= REG_NUM(channelNumber);
    data_to_write[1] = newValue;

    msgs[0].addr  = slaveAddress;  // slave addr for write
    msgs[0].len   = 2;             // two byte message
    msgs[0].flags = 0;             // simple write operation
    msgs[0].buf   = data_to_write;

    msgset.nmsgs  = 1;             // single write message
    msgset.msgs   = msgs;

    if (ioctl(file, I2C_RDWR, (unsigned long)&msgset) < 0)
    {
        printf("error I2C_RDWR errno=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    data_read[6] = 0;

    printf("Transaction complete\n");

    return 0;
}
