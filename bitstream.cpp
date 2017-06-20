#include "bitstream.h"
#include <string>
#include <fstream>
using namespace std;
 
#include <iostream>
 
const int FULLWORD = 8;  // 8 bits per char to file
 
 
void InBitStream::_clear() {
  buffer = 0;
  bufsize = 0;
}
 
InBitStream::InBitStream() {
  _clear();
}
 
InBitStream::~InBitStream() {
  close();
}
 
 
bool InBitStream::isOpen() const {
  return ((const_cast<InBitStream*>(this))->file.is_open());
}
 
bool InBitStream::eof() const {
  return (bufsize==0 && file.eof());
}
 
bool InBitStream::open(const std::string& filename) {
  if (isOpen())
    close();
  file.open(filename.c_str(), ios::binary|ios::in);
  _clear();
  _prefetch();
  return isOpen();
}
 
void InBitStream::_prefetch() {
  if (isOpen() && bufsize==0) {
    // need more from file
    buffer = file.get();
    if (file.eof())
      buffer = 0;
    else
      bufsize=FULLWORD;
  }
}
     
 
int InBitStream::read(int numbits) {
  if (isOpen()) {
    int result = 0;
    if (numbits==1) {
      // just need one bit
      if (bufsize>0) {  // prefetch may have reached eof
    result = (buffer>>(bufsize-1));
    buffer -= result<<(bufsize-1);
    bufsize--;
    if (bufsize==0)
      _prefetch();
      }
    } else {
      int i;
      for (i=0; i<numbits; i++)
    result = (result<<1) + read(1);   // use recursion
    }
    return result;
  }
  return -1;
}
 
 
void InBitStream::close() {
  // must flush if leftover in buffer
  if (isOpen()) {
    file.close();
    file.clear();  // required to clear eof bit in case of reopen
  }
}
 
void InBitStream::next_start_code() {
    // padding to byte aligned
    if(bufsize < 8) read(bufsize);
    // while nextbits() != 0x000001, skip next byte
    while(nextbits()>>8 != 0x000001) read(8);
}
 
int InBitStream::nextbits() {
    int ret = buffer;
    for(int i = 0; i < 3; i++) ret = (ret<<8)+file.get();
    ret = (ret<<8-bufsize)+((file.get()>>bufsize)&(1<<8-bufsize)-1);
    for(int i = 0; i < 4; i++) file.unget();
    return ret;
}
 
int InBitStream::nextbyte() {
    int ret = buffer;
    ret = (ret<<8-bufsize)+(file.get()>>bufsize);
    file.unget();
    return ret;
}