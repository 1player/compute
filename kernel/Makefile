MODULES := lib scheduler

CC := gcc
CFLAGS := -std=gnu17 -MMD -MP -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -I $(CURDIR)

SOURCES := $(wildcard *.c)
include $(patsubst %,%/Makefile.module,$(MODULES))

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

das.kernel: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) das.kernel

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
