#include <stdio.h>
#include <assert.h>

#include "bit_array.h"

namespace libpetey {

bit_array::bit_array() {
  nbits=0;
  nwords=1;
  data=new word[1];
  data[0]=0;
}

bit_array::bit_array(long n) {
  nbits=n;
  nwords=(n-1)/(sizeof(word)*8)+1;
  data=new word[nwords];
}

bit_array::bit_array(long n, char value) {
  nbits=n;
  nwords=(n-1)/(sizeof(word)*8)+1;
  data=new word[nwords];
  if (value == 0) {
    for (long i=0; i<nwords; i++) data[i]=0;
  } else {
    for (long i=0; i<nwords; i++) data[i]=255;
  }
}

bit_array::bit_array(char *d, long n) {
  nbits=n;
  nwords=(n-1)/(sizeof(word)*8)+1;
  for (long i=0; i<n; i++) {
    if (d[i] > 0) on(i); else off(i);
  }
}

bit_array::bit_array(word *d, long nw, long nb) {
  assert(nb<=sizeof(word)*8);
  nwords=nw;
  nbits=nb;
  data=new word[nw];
  for (long i=0; i<nw; i++) data[i]=d[nw-i-1];
}

bit_array::~bit_array() {
  delete [] data;
}

void bit_array::resize (long n) {
  long k;
  long nwn;
  word *new_data;

  nbits=n;

  //calculate new number of words:
  nwn=(n-1)/(sizeof(word)*8)+1;

  //if this is the same as the old, don't need to do anything:
  if (nwn == nwords) return;

  //create extended or truncated array and copy data to it:
  new_data=new word[nwn];
  if (nwn > nwords) {
    for (long i=0; i<nwords; i++) new_data[i]=data[i];
    for (long i=nwords; i<nwn; i++) new_data[i]=0;
  } else {
    for (long i=0; i<nwn; i++) new_data[i]=data[i];
  }

  nwords=nwn;
  nbits=n;

  delete [] data;
  data=new_data;

}

char bit_array::on(long ind) {
  long wordind;
  long bitind;
  word tmpl;
 
  //check for out of bounds subscripts:
  if (ind < 0) {
    printf("Warning: negative subscript encountered\n");
    return -1;
  }
  if (ind >= nbits) resize((ind/nbits+1)*ind);

  //calculate the subscript for the word and subscript for its bit:
  wordind=ind/(sizeof(word)*8);
  bitind=ind % (sizeof(word)*8);

  //left shift a bit to the correct position and do a bitwise or:
  tmpl=1 << bitind;
  data[wordind]=data[wordind] | tmpl;

  return 1;
}

char bit_array::off(long ind) {
  long wordind;
  long bitind;
  word tmpl;
  
  //check for out of bounds subscripts:
  if (ind < 0) {
    printf("Warning: negative subscript encountered\n");
    return -1;
  }
  if (ind >= nbits) resize((ind/nbits+1)*ind);

  //calculate the subscript for the word and subscript for its bit:
  wordind=ind/(sizeof(word)*8);
  bitind=ind % (sizeof(word)*8);

  //left shift a bit to the correct position and do a bitwise and of its complement:
  tmpl=1 << bitind;
  data[wordind]=data[wordind] & ~tmpl;

  return 0;
}

char bit_array::flip(long ind) {
  long wordind;
  long bitind;
  word tmpl, tmpl2;
  
  //check for out of bounds subscripts:
  if (ind < 0) {
    printf("Warning: negative subscript encountered\n");
    return -1;
  }
  if (ind >= nbits) resize((ind/nbits+1)*ind);

  //calculate the subscript for the word and subscript for its bit:
  wordind=ind/(sizeof(word)*8);
  bitind=ind % (sizeof(word)*8);

  tmpl=1 << bitind;
  data[wordind]=data[wordind] ^ tmpl;
}

char bit_array::operator [] (long ind) {
  long wordind;
  long bitind;
  word result1;
  
  //check for out of bounds subscripts:
  if (ind < 0) {
    printf("Warning: negative subscript encountered\n");
    return -1;
  }
  if (ind >= nbits) {
    printf("Warning: out-of-bounds subscript encountered\n");
    return -1;
  }

  //find the location of the bit:
  wordind=ind/(sizeof(word)*8);
  bitind=ind % (sizeof(word)*8);

  //right shift it until it is in the lowest order position:
  result1=data[wordind] >> bitind;

  //taking the modolo wrt 2 will get it:
  return char (result1 % 2);

}

long bit_array::nnonzero(long ind) {
  long wordind;
  long bitind;
  word tw;
  long n;

  //check for out of bounds subscripts:
  if (ind < 0) {
    printf("Warning: negative subscript encountered\n");
    return -1;
  }
  if (ind >= nbits) {
    printf("Warning: out-of-bounds subscript encountered\n");
    return -1;
  }

  //find the location of the bit:
  wordind=ind/(sizeof(word)*8);
  bitind=ind % (sizeof(word)*8);

  n=0;
  for (long i=0; i<wordind; i++) {
    tw=data[i];
    for (int j=1; j<sizeof(word)*8; j++) {
      n+=tw % 2;
      tw = tw >> 1;
    }
    n+=tw % 2;
  }

  tw=data[wordind];
  for (long i=0; i<bitind; i++) {
    n+=tw % 2;
    tw = tw >> 1;
  }
  n+=tw % 2;

  return n;
}

void bit_array::print() {
  for (long i=0; i<nbits; i++) printf("%d ", (*this)[i]);
  //for (long i=0; i<nwords; i++) printf("%b", data[i]);
  printf("\n");
}

}
