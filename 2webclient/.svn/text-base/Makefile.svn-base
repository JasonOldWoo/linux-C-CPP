TARGET=agent
LIBS=libDeviceSDK-x86.a libFastUdx.a
SRCS=deviceagent.cpp
OBJS=deviceagent.o
CFLAGS=-g
LFLAGS=-lpthread
CC=g++

all : $(OBJS) $(TARGET)

$(OBJS) : $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET) : $(OBJS) $(LIBS)
	$(CC) $^ $(LFLAGS) -o $@

.PHONY:clean
clean:
	rm -fr $(TARGET) $(OBJS)
