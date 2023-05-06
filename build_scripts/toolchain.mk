
TOOLCHAIN_PREFIX = $(abspath ../.toolchains/$(TARGET))
export PATH := $(TOOLCHAIN_PREFIX)/bin:$(PATH)

toolchain: toolchain_binutils toolchain_gcc

BINUTILS_SRC = ../.toolchains/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD = ../.toolchains/binutils-build-$(BINUTILS_VERSION)

toolchain_binutils: $(TOOLCHAIN_PREFIX)/bin/i686-elf-ld

$(TOOLCHAIN_PREFIX)/bin/i686-elf-ld: $(BINUTILS_SRC).tar.xz
	cd toolchain && tar -xf binutils-$(BINUTILS_VERSION).tar.xz
	mkdir $(BINUTILS_BUILD)
	cd $(BINUTILS_BUILD) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../binutils-$(BINUTILS_VERSION)/configure \
		--prefix="$(TOOLCHAIN_PREFIX)"	\
		--target=$(TARGET)				\
		--with-sysroot					\
		--disable-nls					\
		--disable-werror
	$(MAKE) -j8 -C $(BINUTILS_BUILD)
	$(MAKE) -C $(BINUTILS_BUILD) install

$(BINUTILS_SRC).tar.xz:
	mkdir -p ../toolchains 
	cd toolchain && wget $(BINUTILS_URL)


GCC_SRC = ../.toolchains/gcc-$(GCC_VERSION)
GCC_BUILD = ../.toolchains/gcc-build-$(GCC_VERSION)

toolchain_gcc: $(TOOLCHAIN_PREFIX)/bin/i686-elf-gcc

$(TOOLCHAIN_PREFIX)/bin/i686-elf-gcc: $(TOOLCHAIN_PREFIX)/bin/i686-elf-ld $(GCC_SRC).tar.xz
	cd toolchain && tar -xf gcc-$(GCC_VERSION).tar.xz
	mkdir $(GCC_BUILD)
	cd $(GCC_BUILD) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../gcc-$(GCC_VERSION)/configure \
		--prefix="$(TOOLCHAIN_PREFIX)" 	\
		--target=$(TARGET)				\
		--disable-nls					\
		--enable-languages=c,c++		\
		--without-headers
	$(MAKE) -j8 -C $(GCC_BUILD) all-gcc all-target-libgcc
	$(MAKE) -C $(GCC_BUILD) install-gcc install-target-libgcc
	
$(GCC_SRC).tar.xz:
	mkdir -p toolchain
	cd ../.toolchains && wget $(GCC_URL)

#
# Clean
#
clean-toolchain:
	rm -rf $(GCC_BUILD) $(GCC_SRC) $(BINUTILS_BUILD) $(BINUTILS_SRC)

clean-toolchain-all:
	rm -rf toolchain/*

.PHONY:  toolchain toolchain_binutils toolchain_gcc clean-toolchain clean-toolchain-all
