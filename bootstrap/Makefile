CC := gcc
LD := gcc


CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)
# Temporarily to make sure function pointers are not considered NativeIntegers
# We can remove once we store closure objects inside method slots
CFLAGS += -falign-functions=16
LDFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)

SOURCES := $(wildcard *.c) $(wildcard builtins/*.c) $(wildcard lib/*.c)
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
