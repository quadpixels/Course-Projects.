/* bitarray.h
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

#ifndef bitarray_h
#define bitarray_h

#include "basic.h"

class BitRankF {
private:
  bool owner;
  ulongBV n,integers;
  ulongBV factor,b,s;
  ulongBV *Rs; //superblock array
  ucharBV *Rb; //block array
  ulongBV BuildRankSub(ulongBV ini,ulongBV fin); //internal use of BuildRank
  void BuildRank(); //crea indice para rank
public:
  ulongBV *data; //here is the bit-array
  BitRankF(ulongBV *bitarray, ulongBV n, bool owner);
  ~BitRankF(); //destructor
  bool IsBitSet(ulongBV i);
  ulongBV rank(ulongBV i); //Rank from 0 to n-1
  ulongBV prev(ulongBV start); // gives the largest index i<=start such that IsBitSet(i)=true
  ulongBV select(ulongBV x); // gives the position of the x:th 1.
  ulongBV SpaceRequirementInBits();
  /*load-save functions*/
  int save(FILE *f);
  int load(FILE *f);
  ulongBV getSize() {return n;}
  BitRankF(FILE *f, int *error);
  BitRankF() {};
};


class BitSelectNext {
private:
	ulongBV *datos; //arreglo de bits
	bool owner;
	ulongBV n;
	ulongBV integers;
public:
	// Crea arreglo segun numero de bits, semilla aleatoria y probabilidad
	BitSelectNext(ulongBV *bit, ulongBV n, bool owner); 
	~BitSelectNext(); //destructor
	ulongBV select_next(ulongBV i); // select_next
};

class BitRankFSparse {
private:
  BitRankF *block;
  BitRankF *sblock;
  ulongBV L;
public:
  BitRankFSparse(ulongBV *bitarray, ulongBV n);
  ~BitRankFSparse(); //destructor
  bool IsBitSet(ulongBV i);
  ulongBV rank(ulongBV i); //Rank from 0 to n-1
  ulongBV prev(ulongBV start); // gives the largest index i<=start such that IsBitSet(i)=true
  ulongBV select(ulongBV x); // gives the position of the x:th 1.
  ulongBV SpaceRequirementInBits();
  int save(FILE *f);
  int load(FILE *f);
  BitRankFSparse(FILE *f, int *error);
};

#endif
