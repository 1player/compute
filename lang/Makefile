CC := gcc
CFLAGS := -std=gnu17 -MMD -MP -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter

SOURCES := $(wildcard *.c)

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

das.lang: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) das.lang

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
