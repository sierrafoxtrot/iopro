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
#define DEFAULT_DEVICE_ADDR 0x10

using namespace std;

// Registers have the type in the upper nibble.
#define REG_TYPE(X) (((X) & 0xF0) >> 4)
#define REG_TYPE_ENCODE(X) (((X) & 0x0F) << 4)

// Registers have the index/number in the lower nibble.
#define REG_NUM(X) ((X) & 0x0F)

enum register_types
{
    REG_ID            = 0x00,
    REG_OUTPUT        = 0x01,
    REG_OUTPUT_BITMAP = 0x02,
    REG_INVALID       = 0x0F,
};

void usage(void)
{
    printf("The \"capetool\" utility is a simple commandline tool for testing the Sierra\n");
    printf("Foxtrot intelligent BeagleBone capes. Based on the commandline options, it\n");
    printf("performs the appropriate I2C read/write 0perations to drive the outputs, read\n");
    printf("inputs and display status information from the chosen cape in a stack.\n\n");
    printf("Operation:\n\n");
    printf("        capetool [OPTION]...\n\n");
    printf("        -b BUS_NAME\n");
    printf("           I2C bus device node (eg /dev/i2c-1)\n");
    printf("        -a SLAVE_ADDRESS\n");
    printf("           Cape address (capes 0..3 == 0x10..0x13)\n");
    printf("        -c CHANNEL_NUMBER\n");
    printf("           If driving a single output, this is the Output number 0..n\n");
    printf("           (n is number of outputs on the cape).\n");
    printf("        -r REG_NUMBER\n");
    printf("           If addressing a specific register such as status\n");
    printf("           Registers:     ID = 0 (ID information for the cape)\n");
    printf("                          OUTPUT = 1 (individually addressed output)\n");
    printf("                          OUTPUT_BITMAP = 2 (all outputs addressed as a bitmap)");
    printf("        -v NEW_VALUE\n");
    printf("           New value if writing to the specified register number. Omitting\n");
    printf("           Omittingthis option, implies a read.\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msgs[2];
    int opt;
    uint8_t slaveAddress = DEFAULT_DEVICE_ADDR;
    char data_read[8];
    char *busName = 0;
    uint8_t channelNumber = 0xFF;
    uint8_t newValue = 0xFF;
    uint8_t registerNumber = REG_OUTPUT;

    while ((opt = getopt(argc, argv, "a:b:c:v:r:"))>0)
    {
        switch (opt) {
        case '?': puts("Bad argument\n\n"); usage(); break;
        case 'b': busName = (char *)malloc(strlen(optarg));
            strcpy(busName, optarg); break;
        case 'a': slaveAddress = atoi(optarg); break;
        case 'c': channelNumber = atoi(optarg); break;
        case 'v': newValue = atoi(optarg); break;
        case 'r': registerNumber = atoi(optarg); break;
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
    data_to_write[0] = REG_TYPE_ENCODE(registerNumber);
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
