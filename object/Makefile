CC := gcc
CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter

SOURCES := $(wildcard *.c)

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

das.object: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) das.object

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
