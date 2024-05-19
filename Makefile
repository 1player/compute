MODULES := object lang kernel

CC := gcc
LD := gcc

CFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)
LDFLAGS := -g -std=gnu17 -MMD -MP -Wall -Wextra -I $(CURDIR)

BUILDDIR := $(CURDIR)/_build

SOURCES := $(wildcard *.c)
include $(patsubst %,%/Makefile.module,$(MODULES))

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

TARGETS := $(patsubst bin/%,$(BUILDDIR)/%,$(wildcard bin/*))

##

all: $(TARGETS)

$(TARGETS): $(BUILDDIR)/%: $(OBJECTS) bin/%/main.o | $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILDDIR):
	mkdir $(BUILDDIR)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) $(BUILDDIR)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

-include $(DEPENDS)
