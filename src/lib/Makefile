include ../Makefile.rules

../lib.a: $(OBJECTS)
	$(AR) -rv $@ $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) ../lib.a

.PHONY: clean

-include $(DEPENDS)
