CC = gcc
LD = gcc

CFLAGS = -std=gnu18 -MMD -MP
CFLAGS += -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter
LFLAGS = $(CFLAGS) -L/usr/lib/

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPENDS = $(patsubst %.c,%.d,$(SOURCES))

DEBUG = yes

ifeq ($(DEBUG), yes)
	CFLAGS += -g
endif

all: dasc

dasc: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o dasc

clean:
	rm -rf $(OBJECTS) $(DEPENDS) dasc

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPENDS)

.PHONY: all clean
