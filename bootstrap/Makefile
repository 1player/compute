CC := gcc
LD := gcc

CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)
LDFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)

SOURCES := $(wildcard *.c) $(wildcard builtins/*.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

##

all: bootstrap

bootstrap: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f bootstrap $(OBJECTS) $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
