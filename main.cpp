#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

#define DEBUG 0
int main(int argc, char* argv[]){
	if(argc != 2){
		printf("need an input MPEG file\n");             
  		return 0;
    }
    printf("%s\n", argv[1]);
    if(decode_init(argv[1], DEBUG)) decode_video_sequence();
	return 0;
}
