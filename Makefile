all:
	make -C kernel
	make -C lang
	make -C object


clean:
	make -C kernel clean
	make -C lang clean
	make -C object clean

.PHONY: all clean
