all:decoder

decoder:main.cpp bitstream.cpp parser.cpp util.cpp 
	g++ -std=c++11 -O2 main.cpp bitstream.cpp parser.cpp util.cpp -o decoder -lm 
run:
	./decoder MPEG/IPB_ALL.M1V
clean:
	rm decoder *.bmp
