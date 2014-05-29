include ../makefile.def

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
TARGET=mh_chat_client

all:$(OBJS) $(TARGET)

%.o:%.c
	$(CC) $^ $(CCFLAGS) -o $@

$(TARGET):$(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

run:all
	./$(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)
