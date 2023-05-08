obj-m += telephone_directory_module.o
telephone_directory_module-objs += telephone_directory.o module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=${PWD} modules
clean:
	-C /lib/modules/$(shell uname -r)/build M=${PWD} clean
