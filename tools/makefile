CC = gcc
CFLAGS = -Wall -fpermissive 


all: clean checkRoot loader reverseShell copy
	@echo "Files created"

checkRoot:
	rm -f ./checkRoot
	$(CC) -o checkRoot checkRoot.c $(CFLAGS)

loader: 
	rm -f ./reflectiveLoader
	$(CC) -o reflectiveLoader reflectiveLoader.c $(CFLAGS)

reverseShell: 
	rm -f ./reverseShell	
	$(CC) -o reverseShell reverseShell.c $(CFLAGS)

clean:
	rm -f ./reflectiveLoader
	rm -f ./checkRoot
	rm -f ./reverseShell
	@echo files clean

copy:
	mkdir -p ../build
	cp -f ./reflectiveLoader ../build
	cp -f ./checkRoot ../build
	cp -f ./reverseShell ../build