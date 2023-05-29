include ../toolchain.mk

SOURCES_C := $(wildcard *.c)		
OBJECTS_C := $(patsubst %.c, %.c.o, $(SOURCES_C))

all: $(OBJECTS_C) pack

%.c.o: %.c
	@../$(CC) $(CFLAGS) -I ../include/ -c $< -o $@
	@echo "--> LIB Compiled: " $<
	
pack:
	@../$(AR) $(ARFLAGS) ../$(BUILD_DIR)/libxnix.a $(OBJECTS_C)
	@rm -rf *.c.o
