include ../makefile.def

LIBSPATH=../../lib
LIBS=libmhthread
TARGET=$(LIBS).so $(LIBS).a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all:$(OBJS) $(TARGET)

%.o:%.c
	$(CC) $< $(CCFLAGS) -o $@

$(LIBS).so:$(OBJS)
	$(CC) -Wall -shared $^ -o $@
	cp -dpR $(LIBS).so $(LIBSPATH)/

$(LIBS).a:$(OBJS)
	ar r $@ $^

clean:
	$(RM) $(OBJS) $(TARGET)
