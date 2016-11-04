# ------------------------------------------------------------------------------
# Makefile32.linux
# Linux Makefile for 32-bit Pinguino
# Regis Blanchot <rblanchot@gmail.com> 
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# directories
# ------------------------------------------------------------------------------
# HOME, PDEDIR, PROC & BOARD are defined in pinguino.py

TARGET		=	firmware

BOARD		=	PIC32_PINGUINO_220
PROC		=	32MX220F032D

HOME		=	$(PWD)/..
OSDIR		=	linux
SRCDIR		=	$(HOME)/firmware
P32DIR		=	$(HOME)/p32
P32CORE		=	$(HOME)/$(OSDIR)/p32
BINDIR		=	$(P32CORE)/bin
INCDIR		=	$(P32DIR)/include
LKRDIR		=	$(P32DIR)/lkr/$(BOARD)
OBJDIR		=	$(P32DIR)/obj/non-free

INCLUDEDIRS	=	-I$(INCDIR)/non-free\
			-I$(INCDIR)/pinguino/core\
			-I$(INCDIR)/pinguino/libraries\
			-I$(LKRDIR)\
			-I$(PDEDIR)\
			-I$(OBJDIR)

LIBDIRS		=	-L$(OBJDIR)/usb

# ------------------------------------------------------------------------------
# unsupported record in hex file / Jean-pierre Mandon 2012
# ------------------------------------------------------------------------------

BADRECORD='^:040000059D006000FA'
ifeq ($(BOARD),PIC32_PINGUINO_220)
	BADRECORD = '^:040000059D0040001A'
endif
ifeq ($(BOARD),GENERIC32MX250F128)
	BADRECORD = '^:040000059D0040001A'
endif
ifeq ($(BOARD),GENERIC32MX220F032)
	BADRECORD = '^:040000059D0040001A'
endif

REMOVE = grep --binary --invert-match $(BADRECORD) $(SRCDIR)/$(TARGET).hex > $(SRCDIR)/temp.hex

# ------------------------------------------------------------------------------
# heap size / Regis Blanchot 2012
# ------------------------------------------------------------------------------

HEAP_SIZE=16384
ifeq ($(BOARD),PIC32_PINGUINO_220)
	HEAP_SIZE = 512
endif
ifeq ($(BOARD),GENERIC32MX220F032)
	HEAP_SIZE = 512
endif

# ------------------------------------------------------------------------------
# select the cdc library / Jean-pierre Mandon 2012
# ------------------------------------------------------------------------------

CDCLIBRARY = libcdc.a
ifeq ($(BOARD),PIC32_PINGUINO_220)
	CDCLIBRARY = libcdc220.a
endif
ifeq ($(BOARD),GENERIC32MX250F128)
	CDCLIBRARY = libcdc220.a
endif
ifeq ($(BOARD),GENERIC32MX220F032)
	CDCLIBRARY = libcdc220.a
endif

# ------------------------------------------------------------------------------
# commands
# ------------------------------------------------------------------------------

CROSS_COMPILE	=	/opt/CodeSourcery/Sourcery_G++_Lite/bin/mips-sde-elf-
CC		=	$(CROSS_COMPILE)gcc
OBJC		=	$(CROSS_COMPILE)objcopy
SIZE		=	$(CROSS_COMPILE)size
LIBS		=	-lm -lgcc -lc
RM		=	rm -f
CP		=	cp
MV		=	mv
PROG		=	$(HOME)/mphidflash/mphidflash

# ------------------------------------------------------------------------------
# flags
# ------------------------------------------------------------------------------

#-fdollars-in-identifiers for ISRwrapper.S
DEFS		=	-D __PIC32MX__ -D __$(PROC)__ -D $(BOARD)

AFLAGS		=	-EL -Os -fdollars-in-identifiers -march=24kc -msoft-float -minterlink-mips16 -ffunction-sections -fdata-sections $(DEFS) $(INCLUDEDIRS)
CFLAGS		=	-EL -Os -fdollars-in-identifiers -march=24kc -msoft-float -minterlink-mips16 -ffunction-sections -fdata-sections $(DEFS) $(INCLUDEDIRS)

LDFLAGS		=	$(LIBDIRS) $(LIBS)

ELF_FLAGS	=	-EL -Wl,--gc-sections\
			-Wl,--defsym,_min_heap_size=$(HEAP_SIZE)\
			-Wl,-Map=$(SRCDIR)/$(TARGET).map\
			-Wl,-L$(OBJDIR)/\
			-Wl,--oformat=elf32-tradlittlemips \
			-T$(LKRDIR)/procdefs.ld\
			-T$(LKRDIR)/elf32pic32mx.x

OBJS		=	$(OBJDIR)/crt0.o \
			$(LKRDIR)/ISRwrapper.o \
			$(SRCDIR)/main.o

#-------------------------------------------------------------------------------
#	rules
#-------------------------------------------------------------------------------

all: clean copy link exec

clean:
	#----------------------------------------------------------------------------
	#	clean
	#----------------------------------------------------------------------------
	$(RM) $(OBJS)
	$(RM) $(SRCDIR)/$(TARGET).elf
	$(RM) $(SRCDIR)/$(TARGET).hex
	$(RM) $(OBJDIR)/processor.o
	$(RM) $(SRCDIR)/$(TARGET).map
	cd $(shell dirname $(PROG)) && make clean

copy:
	#----------------------------------------------------------------------------
	#	copy
	#----------------------------------------------------------------------------
	cp $(OBJDIR)/$(PROC).o $(OBJDIR)/processor.o

link: $(OBJS)
	#----------------------------------------------------------------------------
	#	link
	#----------------------------------------------------------------------------
	$(CC) $(ELF_FLAGS) $(LDFLAGS) $(CFLAGS) -o $(SRCDIR)/$(TARGET).elf \
		$(SRCDIR)/main.o\
		$(OBJDIR)/crt0.o\
		$(OBJDIR)/processor.o\
		$(OBJDIR)/usb/$(CDCLIBRARY)\
		$(OBJDIR)/usb/libadb.a\
		$(LKRDIR)/ISRwrapper.o\
		$(INCDIR)/non-free/p32xxxx.h\
		$(LIBS)
	$(SIZE) $(SRCDIR)/$(TARGET).elf

exec:
	#----------------------------------------------------------------------------
	#	exec
	#----------------------------------------------------------------------------
	$(OBJC) -O ihex $(SRCDIR)/$(TARGET).elf $(SRCDIR)/$(TARGET).hex
	${REMOVE}
	mv $(SRCDIR)/temp.hex $(SRCDIR)/$(TARGET).hex

$(PROG):
	cd $(shell dirname $(PROG)) && make

load: $(PROG) link
	#----------------------------------------------------------------------------
	#	load
	#----------------------------------------------------------------------------
	$(PROG) -r -n -w $(TARGET).hex

%.o: %.S
	#----------------------------------------------------------------------------
	#	compile
	#----------------------------------------------------------------------------
	$(CC) $(AFLAGS) -c -o $@ $^

%.o: %.c
	#----------------------------------------------------------------------------
	#	compile
	#----------------------------------------------------------------------------
	$(CC) $(CFLAGS) -c -o $@ $^
