MODULES = lib.a

include Makefile.rules

all: $(MODULES) das

lib.a:
	$(MAKE) -C lib

das: $(OBJECTS)
	$(CC) $(OBJECTS) $(MODULES) $(CFLAGS) -o das

clean:
	$(MAKE) -C lib clean
	rm -rf $(MODULES) $(OBJECTS) $(DEPENDS) das

.PHONY: all clean

-include $(DEPENDS)
