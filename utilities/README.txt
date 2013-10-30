
To build, (on the BeagleBone)

	 g++ capetool.cpp -o capetool


The "capetool" utility is a simple commandline tool for testing the Sierra
Foxtrot intelligent BeagleBone capes. Based on the commandline options, it
performs the appropriate I2C read/write 0perations to drive the outputs, read
inputs and display status information from the chosen cape in a stack.

Operation:

	capetool [OPTION]...

	-b BUS_NAME
               I2C bus device node (eg /dev/i2c-1)
	-a SLAVE_ADDRESS
	       Cape address (capes 0..3 == 0x10..0x13)
	-c CHANNEL_NUMBER
	       If driving a single output, this is the Output number 0..n
	       (n is number of outputs on the cape).
	-r REG_NUMBER
	       If addressing a specific register such as status
	       Registers:     ID = 0 (ID information for the cape)
                              OUTPUT = 1 (individually addressed output)
                              OUTPUT_BITMAP = 2 (all outputs addressed as a bitmap)

	-v NEW_VALUE
	       New value if writing to the specified channel or register number.
               Omitting this option, implies a read.

Example:
	capetool -b /dev/i2c-1 -c 0 -v 1
		 Turns on relay zero.

	capetool -b /dev/i2c-1 -c 0 -v 0
		 Turns off relay zero.

	capetool -b /dev/i2c-1 -r 2 -v 3
                 Turns on relays 0 & 1 simultaneously via the bitmap register.
