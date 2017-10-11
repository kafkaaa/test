#/*******************************************************************************
#* Copyright (c) 2016
#* All Rights Reserved
#*
#* Description:
#*
#* Author: Nguyen Canh Thang
#*
#* Last Changed By:  $Author: Nguyen Canh Thang
#* Revision:         $Revision: 1.0.0.1 $
#* Last Changed:     $Date:  $
#*
#******************************************************************************/#
#Change gateway below to your desired executible filename

#Set all your object files (the object files of all the .c files in your project, e.g. main.o my_sub_functions.o )
OBJ = board.o buffer.o database.o libemqtt.o main.o mqtt.o spi_gpio.o sx1276.o

#Set any dependant header files so that if they are edited they cause a complete re-compile (e.g. main.h some_subfunctions.h some_definitions_file.h ), or leave blank
DEPS = ./*.h

#Any special libraries you are using in your project (e.g. -lbcm2835 -lrt `pkg-config --libs gtk+-3.0` ), or leave blank
#For Raspberry Pi
LIBS = -lwiringPi -lm -lmariadbclient -lrt

#For Ubuntu
#LIBS = -lwiringPi -lm -lmysqlclient -lrt -lcrypt

#Set any compiler flags you want to use (e.g. -I/usr/include/somefolder `pkg-config --cflags gtk+-3.0` ), or leave blank
CFLAGS = -I/usr/include/mysql

#Set the compiler you are using ( gcc for C or g++ for C++ )
CC = gcc

#Set the filename extensiton of your C files (e.g. .c or .cpp )
EXTENSION = .c

#define a rule that applies to all files ending in the .o suffix, which says that the .o file depends upon the .c version of the file and all the .h files included in the DEPS macro.  Compile each object file
%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#Combine them into the output file
#Set your desired exe output file name here
gateway: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

#Cleanup
.PHONY: clean

clean:
	rm -f *.o *.gch *~ core *~ 
	
#/** End Of File **/
