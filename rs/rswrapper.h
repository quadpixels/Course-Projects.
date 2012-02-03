/*
 *  rswrapper.h
 *  OurIndex
 *
 *  Created by Manish Patil on 4/4/10.
 *  Copyright 2010 LSU. All rights reserved.
 *
 */

#include "bitarray.h"
#include <string>

using namespace std;

class Crankselect
{
private:
	BitRankF *bvrs;
	
public:	
	void build(string inBitVec)
	{
		int n;
		
		n = inBitVec.length();
		unsigned long *bitarray = new unsigned long [n/W+1]; /* cs 20111127 加了1 */
		
		for (uint i = 0; i < n; i++)
			bitclean(bitarray,i);
		
		for (uint i = 0; i < n; i++)
		{
			if (inBitVec[i] == '1')
				bitset(bitarray,i);			
		}
		
		bvrs = new BitRankF (bitarray, n, true);
	}
	
	int getBit(int inPos)
	{
		return bvrs->IsBitSet(inPos);
	}
	
	int getAllocSize()
	{
		return (bvrs->SpaceRequirementInBits()/8);
	}
	
	int rank(int inPos)
	{
		return bvrs->rank(inPos);
	}
	
	int select(int inPos)
	{
		return bvrs->select(inPos);		
	}
	
	int getSize()
	{
		return bvrs->getSize();
	}
	
	void setbit(int inPos)
	{
		bitset(bvrs->data,inPos);			
	}
	
	void cleanbit(int inPos)
	{
		bitclean(bvrs->data, inPos);
	}
};
