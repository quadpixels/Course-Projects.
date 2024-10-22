/* bitarray.cpp
   Copyright (C) 2005, Rodrigo Gonzalez, all rights reserved.

   New RANK, SELECT, SELECT-NEXT and SPARSE RANK implementations.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "bitarray.h"
#include "assert.h"
#include "math.h"
#include <sys/types.h>
#include <string>
#include <iostream>

using namespace std;

/////////////
//Rank(B,i)//
/////////////
//This Class use a superblock size of 256 bits
//and a block size of 32 bits also
//we use a 32 bit to represent Rs and
//we use a 8 bit to represent Rb

// The bitarray goes form 0..n-1

BitRankF::BitRankF(ulongBV *bitarray, ulongBV n, bool owner) {
	data=bitarray;
	this->owner = owner;
	this->n=n;
	b=32; // b is a word
	factor=8; // 8 word in 256 bits
	s=b*factor;
	ulongBV aux=(n+1)%W;
	if (aux != 0)
		integers = (n+1)/W+1;
	else
		integers = (n+1)/W;
	BuildRank();
}

BitRankF::~BitRankF() {
  delete [] Rs;
  delete [] Rb;
  if (owner) delete [] data;
}

ulongBV BitRankF::SpaceRequirementInBits() {
  return (owner?n:0)+(n/s)*sizeof(ulongBV)*8 +(n/b)*sizeof(ucharBV)*8+sizeof(BitRankF)*8; 
}
//Build the rank (blocks and superblocks)
void BitRankF::BuildRank(){
  ulongBV num_sblock = n/s;
  ulongBV num_block = n/b;
  Rs = new ulongBV[num_sblock+1];//+1 we add the 0 pos
  Rb = new ucharBV[num_block+1];//+1 we add the 0 pos
  ulongBV j;
  Rs[0]=0;
  for (j=1;j<=num_sblock;j++)
    Rs[j]=BuildRankSub((j-1)*factor,factor)+Rs[j-1];

  Rb[0]=0;
  for (ulongBV k=1;k<=num_block;k++) {
    j = k / factor;
    Rb[k]=BuildRankSub(j*factor,k%factor);
  }
}

int BitRankF::save(FILE *f) {
  if (f == NULL) return 20;
  if (fwrite (&n,sizeof(ulongBV),1,f) != 1) return 21;
  if (fwrite (data,sizeof(ulongBV),n/W+1,f) != n/W+1) return 21;
  if (fwrite (Rs,sizeof(ulongBV),n/s+1,f) != n/s+1) return 21;
  if (fwrite (Rb,sizeof(ucharBV),n/b+1,f) != n/b+1) return 21;
  return 0;
}

int BitRankF::load(FILE *f) {
  if (f == NULL) return 23;
  if (fread (&n,sizeof(ulongBV),1,f) != 1) return 25;
  b=32; // b is a word
  factor=8; // 8 word in 256 bits
  s=b*factor;
  ulongBV aux=(n+1)%W;
  if (aux != 0)
    integers = (n+1)/W+1;
  else
    integers = (n+1)/W;
  data= new ulongBV[n/W+1];
  if (!data) return 1;
  if (fread (data,sizeof(ulongBV),n/W+1,f) != n/W+1) return 25;
  this->owner = true;
  Rs= new ulongBV[n/s+1];
  if (!Rs) return 1;
  if (fread (Rs,sizeof(ulongBV),n/s+1,f) != n/s+1) return 25;
  Rb= new ucharBV[n/b+1];
  if (!Rb) return 1;
  if (fread (Rb,sizeof(ucharBV),n/b+1,f) != n/b+1) return 25;
  return 0;
}

BitRankF::BitRankF(FILE *f, int *error) {
  *error = BitRankF::load(f);
}

ulongBV BitRankF::BuildRankSub(ulongBV ini,ulongBV bloques){
  ulongBV rank=0,aux;
  for(ulongBV i=ini;i<ini+bloques;i++) {
    if (i < integers) {
      aux=data[i];
      rank+=popcount(aux);
    }
  }
  return rank; //return the numbers of 1's in the interval
}

//this rank ask from 0 to n-1
ulongBV BitRankF::rank(ulongBV i) {
  ++i; // the following gives sum of 1s before i
  return Rs[i>>8]+Rb[i>>5]
    +popcount(data[i>>5] & ((1<<(i & mask31))-1));
}

ulongBV BitRankF::prev(ulongBV start) {
      // returns the position of the previous 1 bit before and including start.
      // tuned to 32 bit machine

      ulongBV i = start >> 5;
      int offset = (start % W);
      ulongBV answer = start;
      ulongBV val = data[i] << (Wminusone-offset);

      if (!val) { val = data[--i]; answer -= 1+offset; }

      while (!val) { val = data[--i]; answer -= W; }

      if (!(val & 0xFFFF0000)) { val <<= 16; answer -= 16; }
      if (!(val & 0xFF000000)) { val <<= 8; answer -= 8; }

      while (!(val & 0x80000000)) { val <<= 1; answer--; }
      return answer;
}

ulongBV BitRankF::select(ulongBV x) {
  // returns i such that x=rank(i) && rank(i-1)<x or n if that i not exist
  // first binary search over first level rank structure
  // then sequential search using popcount over a int
  // then sequential search using popcount over a char
  // then sequential search bit a bit

  //binary search over first level rank structure
  ulongBV l=0, r=n/s;
  ulongBV mid=(l+r)/2;
  ulongBV rankmid = Rs[mid];
  while (l<=r) {
    if (rankmid<x)
      l = mid+1;
    else
      r = mid-1;
    mid = (l+r)/2;
    rankmid = Rs[mid];
  }
  //sequential search using popcount over a int
  ulongBV left;
  left=mid*factor;
  x-=rankmid;
        ulongBV j=data[left];
        ulongBV ones = popcount(j);
        while (ones < x) {
    x-=ones;left++;
    if (left > integers) return n;
          j = data[left];
      ones = popcount(j);
        }
  //sequential search using popcount over a char
  left=left*b;
  rankmid = popcount8(j);
  if (rankmid < x) {
    j=j>>8;
    x-=rankmid;
    left+=8;
    rankmid = popcount8(j);
    if (rankmid < x) {
      j=j>>8;
      x-=rankmid;
      left+=8;
      rankmid = popcount8(j);
      if (rankmid < x) {
        j=j>>8;
        x-=rankmid;
        left+=8;
      }
    }
  }

  // then sequential search bit a bit
        while (x>0) {
    if  (j&1) x--;
    j=j>>1;
    left++;
  }
  return left-1;
}

bool BitRankF::IsBitSet(ulongBV i) 
{
	return bitget(data, i);
//  return (1u << (i % W)) & data[i/W];
}

/* Implementation of  Bitselect Next */
BitSelectNext::BitSelectNext(ulongBV *bit, ulongBV n, bool owner) {
        this->owner = owner;
	this->datos=bit;
	this->n=n;
	ulongBV aux=(n+1)%W;
	if (aux != 0)
		integers = (n+1)/W+1;
	else 
		integers = (n+1)/W;
}

//Select Next
//this selectnext ask from 1 to n
ulongBV BitSelectNext::select_next(ulongBV k) {
	ulongBV count = k-1;
	ulongBV des,aux2;
	des=count%W;
	aux2= datos[count/W] >> des;
	if (aux2 > 0) {
		if ((aux2&0xff) > 0) return count+select_tab[aux2&0xff];
		else if ((aux2&0xff00) > 0) return count+8+select_tab[(aux2>>8)&0xff];
		else if ((aux2&0xff0000) > 0) return count+16+select_tab[(aux2>>16)&0xff];
		else {return count+24+select_tab[(aux2>>24)&0xff];}
	}
	
	for (ulongBV i=count/W+1;i<integers;i++) {
		aux2=datos[i];
		if (aux2 > 0) {
			if ((aux2&0xff) > 0) return i*W+select_tab[aux2&0xff];
			else if ((aux2&0xff00) > 0) return i*W+8+select_tab[(aux2>>8)&0xff];
			else if ((aux2&0xff0000) > 0) return i*W+16+select_tab[(aux2>>16)&0xff];
			else {return i*W+24+select_tab[(aux2>>24)&0xff];}
		}
	}
	return n+1;
}

BitSelectNext::~BitSelectNext() {
  if (owner) delete datos;
}

// The bitarray have positions from 0..n-1
BitRankFSparse::BitRankFSparse(ulongBV *bitarray, ulongBV n){
  ulongBV ones=0;
  ulongBV i,j,*sblockbit,*blockbit;
  //assert(bitget(bitarray,n-1));
  for (i=0;i<n;i++) 
    if (bitget(bitarray,i)) ones++;
  L=(ulongBV)ceil(sqrt(n/ones));
  sblockbit=new ulongBV[(n/L+1)/W+1];
  ulongBV aux=0;
  for (i=0;i<(n/L+1)/W+1;i++) sblockbit[i] = 0;
  for (i=0;i*L<n;i++){
    assert(i*L<n);
    for (j=i*L;j<min(n,(i+1)*L);j++) {
      if (bitget(bitarray,j)) { bitset(sblockbit,i); aux++; break; }
    }
    //printf("maximo j revisado=%d\n", j);
    
  }
  sblock = new BitRankF(sblockbit,n/L+1,true);
  /*blockbit=new ulongBV[(L*sblock->rank(n/L))/W+1];*/
  ulongBV aux_long = (L*sblock->rank(n/L))/W ;
  blockbit=new ulongBV[aux_long+1];
  for (i=0;i<aux_long+1;i++) blockbit[i] = 0;
  ulongBV loc=0;
  for (i=0;i*L<n;i++){
    assert(i*L<n);
    if (bitget(sblockbit,i)) {
      for (j=i*L;j<min(n,(i+1)*L);j++) {
	if (bitget(bitarray,j)) bitset(blockbit,j-i*L+loc);
      }
      loc+=L;
    //  printf("maximo j revisado=%d\n", j);
    }
  }
  block = new BitRankF(blockbit,L*sblock->rank(n/L),true); 

  //printf("largo de bloque %lu\n", L);
  //printf("numero de bloques activos%lu %lu\n", aux, sblock-> rank(n/L));

} 

BitRankFSparse::~BitRankFSparse() {
  delete sblock;
  delete block;
}

bool BitRankFSparse::IsBitSet(ulongBV i){
  ulongBV numblock=i/L;
  if (sblock->IsBitSet(numblock) == false) return false;
  else {
    ulongBV one_blocks=sblock->rank(numblock);
    ulongBV zero_blocks=numblock-one_blocks+1;
    //printf("mmm i=%d, zero_blocks=%d, L=%d, res=%d \n",i,zero_blocks,L, i-zero_blocks*L);
    return block->IsBitSet(i-zero_blocks*L);
  }
}

ulongBV BitRankFSparse::rank(ulongBV i){
  ulongBV numblock=i/L;
  ulongBV one_blocks=sblock->rank(numblock);
  if (sblock->IsBitSet(numblock) == false) {
    return block->rank(one_blocks*L-1);
  } else {
    ulongBV zero_blocks=numblock-one_blocks+1;
    return block->rank(i-zero_blocks*L);
  }
}

ulongBV BitRankFSparse::select(ulongBV i){
  ulongBV pos1=block->select(i);
  ulongBV one_blocks=pos1/L+1;
  ulongBV pos2=sblock->select(one_blocks);
  ulongBV zero_blocks=pos2-one_blocks+1;
  return pos1+zero_blocks*L;
}

ulongBV BitRankFSparse::SpaceRequirementInBits(){
  return block->SpaceRequirementInBits()+sblock->SpaceRequirementInBits()+sizeof(BitRankFSparse)*8;
}

ulongBV BitRankFSparse::prev(ulongBV start) {
      // returns the position of the previous 1 bit before and including start.
      // tuned to 32 bit machine
  ulongBV temp,zero_blocks;
  ulongBV numblock=start/L+1;
  ulongBV one_blocks=sblock->rank(numblock-1);
  zero_blocks=numblock-one_blocks;
  if (sblock->IsBitSet(numblock-1) == false) {
    temp = block->prev(one_blocks*L-1);
    numblock = sblock->prev(numblock-1)+1;
    zero_blocks=numblock-(one_blocks-1)-1;
    //printf("Caso1 ");
    return temp+zero_blocks*L;
  } else {
    temp = block->prev(start-zero_blocks*L);
    if (temp < (one_blocks-1)*L) {
      numblock = sblock->prev(numblock-2)+1;
      zero_blocks=numblock-(one_blocks-1);
    //printf("Caso2 ");
      return temp+zero_blocks*L;
    } else {
      zero_blocks=numblock-(one_blocks-1)-1;
    //printf("Caso3 ");
      return temp+zero_blocks*L;
    }
  }
}

int BitRankFSparse::save(FILE *f) {
  if (f == NULL) return 20;
  if (block->save(f) !=0) return 21;
  if (sblock->save(f) !=0) return 21;
  if (fwrite (&L,sizeof(ulongBV),1,f) != 1) return 21;
  return 0;
}

int BitRankFSparse::load(FILE *f) {
  int error;
  if (f == NULL) return 23;
  block = new BitRankF(f,&error);
  if (error !=0) return error;
  sblock = new BitRankF(f,&error);
  if (error !=0) return error;
  if (fread (&L,sizeof(ulongBV),1,f) != 1) return 25;
  return 0;
}

BitRankFSparse::BitRankFSparse(FILE *f, int *error) {
  *error = BitRankFSparse::load(f);
}


