BUILD	:= $(shell uname)
SYSTEM  := $(BUILD)
CC	 = $(CROSS)gcc
AR	 = $(CROSS)ar
RANLIB	 = $(CROSS)ranlib

CFLAGS   = -Wall -g -I.. -DDEBUG
CFLAGS   += -DCONFIG_DIR=\"$(PWD)/cert\"
LDFLAGS  = -g
LIBS     = -lssl -lcrypto

ifeq (Linux, $(SYSTEM))
APPS     = udpserver udpclient udpchat tcpserver tcpclient tcpsslserver tcpsslclient
endif
ifeq (mingw32, $(SYSTEM))
EXE      = .exe
APPS     = udpserver$(EXE) udpclient$(EXE) udpchat$(EXE) tcpserver$(EXE) tcpclient$(EXE) tcpsslserver$(EXE) tcpsslclient$(EXE)
LIBS     += -lwsock32
endif
ifeq (Darwin, $(SYSTEM))
APPS     = udpserver udpclient udpchat tcpserver tcpclient tcpsslserver tcpsslclient
ifeq (Linux, $(BUILD))
CFLAGS  += -I/opt/i686-apple-darwin10/sysroot/Developer/Headers/FlatCarbon
endif
endif
ifeq (Android, $(SYSTEM))
APPS     = udpserver udpclient udpchat tcpserver tcpclient tcpsslserver tcpsslclient
ACFLAGS += --sysroot ${ANDROID_NDK}/build/platforms/android-4/arch-arm -fPIC -mandroid -DANDROID -DOS_ANDROID
CFLAGS  += $(ACFLAGS)
LDFLAGS += $(ACFLAGS)
endif

all: $(APPS)

udpserver$(EXE): udpserver.o ../udp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

udpclient$(EXE): udpclient.o ../udp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

udpchat$(EXE): udpchat.o ../udp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

tcpserver$(EXE): tcpserver.o ../tcp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

tcpclient$(EXE): tcpclient.o ../tcp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

tcpsslserver$(EXE): tcpsslserver.o ../tcp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

tcpsslclient$(EXE): tcpsslclient.o ../tcp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o ../*.o $(APPS) *.exe
