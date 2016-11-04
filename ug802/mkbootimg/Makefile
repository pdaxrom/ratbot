TARGET = mkbootimg

all: $(TARGET)

OBJS = mkbootimg.o

CFLAGS = -O2 -Wall

LIBS = -lcrypto

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS)
