MODULES := lib

CC := gcc
CFLAGS := -std=gnu17 -MMD -MP -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -I $(CURDIR)

SOURCES := $(wildcard *.c)
include $(patsubst %,%/Makefile.module,$(MODULES))

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

das: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) das

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
