#ifndef BITSTREAMS_H
#define BITSTREAMS_H
 
#include <fstream>
#include <string>
 
class InBitStream {
 public:
  InBitStream();
  ~InBitStream();
 
  // Associates bistream with given filename
  bool open(const std::string& filename);
 
  // Checks whether bitstream is currently open and valid
  bool isOpen() const;
 
  // Read specified number of bits from file
  // (will be padded with trailing zeros if eof reached)
  int read(int numbits=1);  // numbits is at most eight.
 
  // Have we reached end of data
  bool eof() const;
 
  // Closes file
  void close();
 
  void next_start_code();
   
  int nextbits();
   
  int nextbyte();
 
 private:
  std::ifstream file;
  int buffer;
  int bufsize;
   
  void _clear();
  void _prefetch();
 
  // disallow clones
  InBitStream(const InBitStream& ibs);
  InBitStream& operator=(const InBitStream& ibs);
};
 
 
#endif