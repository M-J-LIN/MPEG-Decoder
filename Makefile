#all:decoder GUI

decoder:main.cpp bitstream.cpp parser.cpp util.cpp 
	g++ -std=c++11 -O2 main.cpp bitstream.cpp parser.cpp util.cpp -o decoder -lm 
GUI:ui.cpp parser.cpp util.cpp bitstream.cpp
	g++ -std=c++11 -O2 ui.cpp parser.cpp util.cpp bitstream.cpp -lgdi32 -lkernel32
run:
	./decoder MPEG/IPB_ALL.M1V
clean:
	rm *.exe *.bmp
