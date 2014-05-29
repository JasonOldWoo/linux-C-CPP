CC=gcc

FLAGS=-W -g	\
	-I./include

CCFLAGS=$(FLAGS) -c
CFLAGS=$(FLAGS) -lpthread
RM=rm -rf core.*

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
TARGET=test

all:$(OBJS) $(TARGET)

%.o:%.c
	$(CC) $^ $(CCFLAGS) -o $@

$(TARGET):$(OBJS) $(ADDONSOBJS)
	$(CC) $^ $(CFLAGS) -o $@

run:all
	./$(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)
