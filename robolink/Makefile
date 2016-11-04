CONSOLE		= console
RSYSTEM		= rsystem

all: $(CONSOLE) $(RSYSTEM)

CFLAGS		 = -Wall `pkg-config sdl --cflags` -Ipult/ext -Iplatform/cam -I. -Wno-switch
CFLAGS		+= -g

LDFLAGS		 = `pkg-config sdl SDL_gfx --libs` -lssl -ljpeg
LDFLAGS		+= -g

C_OBJS		 = pult/console.o pult/jpegtoraw.o
C_OBJS		+= lib/aes.o lib/tcp.o lib/udp.o pult/ext/DT_drawtext.o pult/ext/internal.o

R_OBJS		 = platform/rsystem.o platform/yuyvtojpeg.o
R_OBJS		+= lib/aes.o lib/tcp.o lib/udp.o
R_OBJS		+= platform/cam/v4l2uvc.o

$(CONSOLE): $(C_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(RSYSTEM): $(R_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -lpthread

clean:
	rm -f $(C_OBJS) $(CONSOLE) $(R_OBJS) $(RSYSTEM)
