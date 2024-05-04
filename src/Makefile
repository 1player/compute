CC = gcc
LD = gcc

CFLAGS = -Wall -pipe -std=gnu18
OFLAGS = -c -I/usr/include -I.
LFLAGS = $(CFLAGS) -L/usr/lib/

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

DEBUG = yes

ifeq ($(DEBUG), yes)
	CFLAGS += -g
endif

all: dasc

dasc: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o dasc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o dasc

rebuild: clean all

.PHONY : clean
.SILENT : clean
