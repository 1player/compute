CC := gcc
LD := gcc

CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)
LDFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

##

all: kernel

kernel: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f kernel $(OBJECTS) $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
