obj-m += keyboard_handler_module.o

kbuild:
	make -C /lib/modules/$(shell uname -r)/build M=${PWD}
clean:
	-C /lib/modules/$(shell uname -r)/build M=${PWD} clean
