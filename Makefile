all:decoder

decoder:main.cpp bitstream.cpp parser.cpp
	g++ -std=c++11 -O2 main.cpp bitstream.cpp parser.cpp -o decoder -lm 
run:
	./decoder ../MPEG_2016/MPEG/IPB_ALL.M1V
clean:
	rm decoder *.bmp
