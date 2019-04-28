CC = gcc
DB = gdb
CFLAGS = -o
DFLAGS = -g
MAIN = 111044074_main
PROGNAME = tiffProcessor

all:
	$(CC) -std=c11 -c -lm $(MAIN).c
	$(CC) $(MAIN).o $(CFLAGS) $(PROGNAME)

debug:
	$(CC) $(DFLAGS) $(MAIN).c $(CFLAGS) $(PROGNAME)
	$(DB) ./$(PROGNAME)


clean:
	rm -f $(PROGNAME) *.o 
