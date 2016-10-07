include Makefile.inc

SRCS = main.c

OBJS = $(SRCS:.c=.o)

TARGET = pstress

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(TARGET)

pstress: $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	-rm -f $(OBJS) $(TARGET) *~
