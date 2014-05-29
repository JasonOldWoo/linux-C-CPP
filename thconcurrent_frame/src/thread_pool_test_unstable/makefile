SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
CC=gcc
INCLUDES=-I./
LIBS=-L/ -lpthread
CCFLAGS = -g -Wall 
cThreadPool : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)

%.o : %.cpp
	$(CC) -c $<


.PHONY:clean
clean:
	rm -rf *.o cThreadPool core.*
