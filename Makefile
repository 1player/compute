all:
	$(MAKE) -C bootstrap
	$(MAKE) -C kernel

clean:
	$(MAKE) -C bootstrap clean
	$(MAKE) -C kernel clean

.PHONY: all clean
