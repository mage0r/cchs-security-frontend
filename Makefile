CC=gcc
LIBS=-lnfc -lusb -lfreefare -lcurl
LDFLAGS=-L/opt/local/lib
CPPFLAGS=-I/usr/local/include
ACTIONS=actions.c
all: 
	$(CC) door-system.c $(ACTIONS) backend-comms.c -I. -std=c99 -g -O2 -o door-system $(LIBS) $(LDFLAGS) $(CPPFLAGS)
clean:
	rm door-system
