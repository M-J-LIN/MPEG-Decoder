target: done
done: GUI
#	rm *.o
CC = g++
CFLAGS = -static -O2
SRC = main.cpp parser.cpp util.cpp bitstream.cpp
OBJS = $(SRC:.cpp=.o)
SRC_UI = ui.cpp parser.cpp util.cpp bitstream.cpp
OBJS_UI = $(SRC_UI:.cpp=.o)

LDFLAGS = -lgdi32 -lkernel32

all:decoder GUI

decoder:$(OBJS)
	$(CC) $(CFLAGS) -c -o parser.o parser.cpp
	$(CC) $(OBJS) -o $@
GUI:$(OBJS_UI)
	$(CC) $(CFLAGS) -c -o ui.o ui.cpp
	$(CC) $(OBJS_UI) -o GUI $(LDFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<
run:
	./decoder MPEG/IPB_ALL.M1V
clean:
	rm *.o *.exe *.bmp
