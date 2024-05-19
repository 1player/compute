MODULES := object lang kernel

CC := gcc
LD := gcc

CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)
LDFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)

SOURCES := $(wildcard *.c)
include $(patsubst %,%/Makefile.module,$(MODULES))

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

TARGETS := $(patsubst bin/%,%,$(wildcard bin/*))

##

all: $(TARGETS)

$(TARGETS): %: $(OBJECTS) bin/%/main.o
	mkdir -p _build
	$(LD) $(LDFLAGS) -o _build/$@ $^

clean:
	rm -rf $(OBJECTS) $(DEPENDS) $(patsubst %,_build/%,$(TARGETS))

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
