CFLAGS =-DDEBUG
# build vcpt executable when user executes "make"
#cc -g vcpt.c -l sqlite3 -lcurl

vcpt: vcpt.o
	$(CC) $(LDFLAGS) vcpt.o -o vcpt -l sqlite3 -lcurl 
vcpt.o: vcpt.c
	$(CC) $(CFLAGS) -c vcpt.c 
# remove object files and executable when user executes "make clean" 
clean: rm *.o vcpt 