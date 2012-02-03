/* Sui Chen CSC7300
 * Dr. Rahul Shah
 * 11-21-11
 * 12-01-11: Fixed a bug in the wavelet tree.
 * Search Functions
 */

#include "godhelpme.h"
#include <sstream>
#include <string>

// 11-27-11: Change this directive to enable/disable wavelet tree acceleration.
#define WAVELET_TREE

/* b/c we are processing charstream so it should not exceed 256.
   for unicode, this should be increased to 65536. */
#define BUCKET_SIZE 256

/* Blacklist: we should treat # as "1" while sorting. 
 * This will influence the order of the dictionary.
 */
#define BLACKLIST_SIZE 1
char bl_c[]={'#'};
int  bl_i[]={1};

/* Change these for debugging information. */
//#define DEBUG 1
//#define DEBUG_WLT

int val(int a) {
	int i;
	for(i=0; i<BLACKLIST_SIZE; i++) {
		if(a==bl_c[i])return bl_i[i];
	}
	return a;
}

int* alphabetical_sort(int* array, int len) {
	int i, ctr[BUCKET_SIZE]; for(i=0;i<BUCKET_SIZE;i++)ctr[i]=0;
	int* ret = (int*)malloc(sizeof(int)*len); for(i=0;i<len;i++)ret[i]=0;
	for(i=0; i<len; i++) {
		int v = val(array[i]);
		ctr[v]++;
	}
//	for(i=0; i<BUCKET_SIZE; i++)printf("ctr[%d]=%d\n",i,ctr[i]);
	for(i=1; i<BUCKET_SIZE; i++) ctr[i]=ctr[i]+ctr[i-1];
	for(i=len-1; i>=0; i--) {
		int p=ctr[val(array[i])]-1;
		ret[p]=array[i];
		ctr[val(array[i])]--;
//		printf("%c goes to place %d\n", array[i], p);
	}
	return ret;
}

// Refer to the PPT. The first and last occurances of needle, within the range of fr and lr
// returns -1 if not found
int findinarray_ff(int* array, int arraylen, int fr, int lr, int needle, struct int2* result) {
	assert(fr>=0 && lr<arraylen);
	int p;
	int first, last; first=-1; last=-1;
	p=fr; while(p<=lr) {
		if(array[p]==needle) { first=p; break;}
		p++;
	}
	p=lr; while(p>=fr) {
		if(array[p]==needle) { last=p; break; }
		p--;
	}
	result->i1=first; result->i2=last;
	if(first==-1) { assert(first==last); return -1;}
	return 0;
}

/* Should have the sorted array first */
/* Calculate the "dictionary and the "C" (number of elements smaller than it) */
int get_dicandc(int* alpha, int len, struct intarray2* result) {
	int i, j, k, first, d=1; //d=number of distinct numbers.
	first=alpha[0];
	for(i=1; i<len; i++) { 
		if(first!=alpha[i]) {assert(val(alpha[i])>val(first)); d++;} //I assert it's dict sorted.
		first=alpha[i];
	}
	assert(d>0);
	result->len1=d;
	result->len2=d;
	/* int array 1 is dictionary */
	int* ia1 = (int*)malloc(sizeof(int)*d); for(i=0;i<d;i++)ia1[i]=0;
	/* int array 2 is C */
	int* ia2 = (int*)malloc(sizeof(int)*d); for(i=0;i<d;i++)ia2[i]=0;
	first=-999; j=-1; //j is used as pointer to C and dict.
	k=0;
	for(i=0; i<len; i++) {
		k++;
		if(first!=alpha[i]) {
			assert(val(alpha[i])>val(first));
			j++;
			ia2[j]=k;
			ia1[j]=alpha[i];
			first=alpha[i];
		}
	}
	for(j=0; j<d; j++) ia2[j]--;
	//ia2[0]=0; /* This is not a counter. shift by 1. */
	
	result->ia1=ia1;
	result->ia2=ia2;
	return 0;
}

/* find an int in an int* */
int index_of(int* haystack, int len, int needle, int islastindex) {
	assert(needle>=0);
	int i; for(i=0; i<len; i++) {
		if(islastindex) {
			if(haystack[len-1-i]==needle)return len-1-i;
		}else{
			if(haystack[i]==needle)return i;
		}
	}
	return -1; /* not found */
}

/* 查看bwt中的第pbwt个元素在alpha中是第几个元素。 */
/* len = length of bwt */
int bwt_to_alpha_mapping(int* bwt, int* alpha, struct intarray2* dictc, 
	int len, int pbwt)
{
	assert(pbwt>=0 && pbwt<len);
	int rk_in_bwt = 1; //在BWT中，这个元素排名第几？
	int needle = bwt[pbwt];
	int j, i = index_of(bwt, pbwt, needle, 1);
	for(i; i>=0; i--)
	{if(bwt[i]==needle)rk_in_bwt=rk_in_bwt+1;}
	j = index_of(dictc->ia1, dictc->len1, needle, 0);
	assert(j!=-1);
	int ret=dictc->ia2[j]+rk_in_bwt-1;
//	printf("rk_in_bwt=%d, dictc->ia2[j]=%d\n", rk_in_bwt, dictc->ia2[j]);
//	printf("BWT[%d]=%c to Alpha[%d]=%c\n", pbwt, bwt[pbwt], ret, alpha[ret]);
	return ret;
}

int bwt_to_alpha_mapping_wltree(int needle, struct wavelettree* wltree, int until) {
	int rk = wavelettree_rank(wltree, until, needle);
	struct intarray2* dictc = wltree->dictc;
	int i = index_of(dictc->ia1, dictc->len1, needle, 0);
	assert(i!=-1);
#ifdef DEBUG
	printf("rk=%d, dictc->ia2[i]=%d\n", rk, dictc->ia2[i]);
#endif
	int ret = dictc->ia2[i] + rk - 1;
	return ret;
}

/* Find the first occurance & last occurance */
int find_num_of_occurances(
	int* alpha,
	int* bwt,
	struct wavelettree* bwt_wlt,
	int arraylen,
	struct intarray2* dictc,
	int* needle,
	int needlen,
	struct int2* result)
{
	assert(needlen>0); assert(dictc->len1==dictc->len2);
	int p = needlen-1; // Needle length
	int ret;
	struct int2 fl_last;
	findinarray_ff(alpha, arraylen, 0, arraylen-1, needle[needlen-1], &fl_last); /* 找到此needle的最后一个字符出现首末 */
																				 /* could be removed; use dicf and dicl only. */
																				 /* This is only the 1st step. */
	/* Let's check with the dic. */
	int dicf = index_of(dictc->ia1, dictc->len1, needle[needlen-1], 0);
	int dicl = -1, dicf1, dicl1;
	struct int2 si2;
	if(dicf!=-1) {
		dicl = (dicf==dictc->len1-1 ? arraylen-1 : dictc->ia2[dicf+1]-1);
		dicf = dictc->ia2[dicf];
	}
//	printf("## %d %d\n$$ %d %d\n", dicf, dicl, fl_last.i1, fl_last.i2);
	assert(dicf==fl_last.i1); assert(dicl==fl_last.i2);
	if(fl_last.i1==-1) {printf("Not found!\n"); return -1;}
	else /* 只找一个字节。而且找到了。 */
	{ si2.i1=dicf; si2.i2=dicl; }
	while(p>0) {
#ifdef DEBUG
		printf(">> Searching for [%c]\n", needle[p-1]);
#endif
		findinarray_ff(bwt, arraylen, dicf, dicl, needle[p-1], &si2);
		if(si2.i1==-1) {
			assert(si2.i2==-1);
			printf(" $$$ Not found!\n");
			return -1;
		}
#ifndef WAVELET_TREE
		dicf = bwt_to_alpha_mapping(bwt, alpha, dictc, arraylen, si2.i1);
		dicl = bwt_to_alpha_mapping(bwt, alpha, dictc, arraylen, si2.i2);
#else
		dicf = bwt_to_alpha_mapping_wltree(needle[p-1], bwt_wlt, si2.i1);
		dicl = bwt_to_alpha_mapping_wltree(needle[p-1], bwt_wlt, si2.i2);
		int dicf_ref = bwt_to_alpha_mapping(bwt, alpha, dictc, arraylen, si2.i1);
		int dicl_ref = bwt_to_alpha_mapping(bwt, alpha, dictc, arraylen, si2.i2);
//		printf("dicf=%d (ref=%d); dicl=%d (ref=%d)\n", dicf, dicf_ref, dicl, dicl_ref);
#endif
//		assert(dicf1==dicf);

#ifdef DEBUG
//		printf("dicf=%d, dicf1=%d\n", dicf, dicf1);
		printf(" bwt(%d,%d) to alpha(%d,%d)\n",
			si2.i1, si2.i2, dicf, dicl);	
#endif
		p--;
	}
	assert(si2.i2>=si2.i1);
//	ret = si2.i2-si2.i1+1;
	ret = dicl - dicf + 1;
#ifdef DEBUG
	printf("Found! occurances=%d\n", dicl - dicf + 1);
#endif
//	result->i1=si2.i1;
//	result->i2=si2.i2;
	result->i1=dicf;
	result->i2=dicl;
	return ret;
}

int* occurances_to_locations(int* sa, struct int2* si2, int arraylen) {
	assert((si2->i2)>=(si2->i1));
	int sz = (si2->i2) - (si2->i1) + 1;
	int* ret = (int*)malloc(sizeof(int)*sz);
	int i,j=0; for(i=(si2->i1); i<=(si2->i2); i++, j++) {
		assert(i<arraylen);
		assert(sa[i]<arraylen);
		int at = (sa[i]-1);
//		printf("%d ", (i+1)%arraylen);
		ret[j]=at;
	}
	printf("\n");
	return ret;
}

/* In place transform of the wavelet tree.
 * 
 * array = the int array representing the string, input.
 * bits  = wavelet tree at a current layer.
 * start & end: bits[start, end) will be changed.
 * le: abbreviation of "less than or equal". characters less than or equal to this will be 0, else 1.
 * 
 * return value: numbers of 0's in this transform.
 * 
 * Example: in this string, chars less than or equal to 'i' will be 0, else 1.
 * rdkadrfs  eiieepklptppPoppPccck   ip  eeeere
 * 10100101000000011111110111000010000100000010
 * 
 */
int wavelettree_inplace_transform(int* array, char* bits, int start, int end, int le) { /* le = less than or equal to。如果小于等于这个就变成0 */
	int i,j,k,zeros_cnt=0,ones_cnt=0;
#ifdef DEBUG
	printf(" >> inplace_transform, start=%d end=%d le=%c : [",  start, end, le);
	for(i=start; i<end; i++) printf("%c", array[i]);
	printf("]\n");
#endif
	if(start==end) return 0;
	int* tmp_array = (int*)malloc(sizeof(int)*(end-start));
	for(i=start; i<end; i++) {
		if(val(array[i]) <= val(le)) /* 注意：我们这里有Black List，所以要用val()比大小。 */
			{bits[i]=0; zeros_cnt++;}
		else 
			{bits[i]=1; ones_cnt++; }
	}
	j=0; k=zeros_cnt;
	for(i=start; i<end; i++) {
		if(bits[i]==0)
			{tmp_array[j]=array[i]; j=j+1;}
		else{tmp_array[k]=array[i]; k=k+1;}
	}
	for(i=0; i<end-start; i++)
		{ /*printf("%c", tmp_array[i]);*/ array[start+i]=tmp_array[i]; }
	free(tmp_array);
	return zeros_cnt;
}

struct wavelettree* wavelettree_build(int* array, int len, struct intarray2* dict) {
	int i, tree_levels=0, j, k;
	struct wavelettree* ret = (struct wavelettree*)malloc(sizeof(struct wavelettree));
	char* bits;		/* The wavelet tree in this level. Each char is either 0 or 1 and fed into Crankselect. */
	char* dic01;	/* to denote the dictionary entry --> [01] translation table. */
	int* curr_limits, *next_limits;
	for(i=0; i<len; i++) assert(index_of(dict->ia1, dict->len1, array[i], 0)!=-1);
	int* array1 = (int*)malloc(sizeof(int)*len); memcpy(array1, array, sizeof(int)*len);
	curr_limits = (int*)malloc(sizeof(int)*len);
	next_limits = (int*)malloc(sizeof(int)*len);
	bits = (char*)malloc(sizeof(char)*len);
	dic01 = (char*)malloc(sizeof(char)*(dict->len1));
	for(i=0; i<len; i++) {curr_limits[i]=0; next_limits[i]=0; }
	for(i=0; i<len; i++) bits[i]='1';
	for(i=0; i<dict->len1; i++)dic01[i]='0';
	i=1; j=0;
	while(i<dict->len1) { i=i*2; j=j+1; }
	tree_levels = j;
	Crankselect** cr = (Crankselect**)malloc(sizeof(Crankselect*)*tree_levels);
	ret->rss = cr;
	ret->level = tree_levels;
	ret->width = len;
	ret->dictc = dict;
	
	Crankselect* curr_cr;
	
	curr_limits[0]=0; /* curr_limits[1]=len-1; */
	curr_limits[1] = len;
	for(i=0; i<tree_levels; i++) {
		for(j=0; j<dict->len1; j++) if((j>>(tree_levels-i-1))&1) dic01[j]='1'; else dic01[j]='0';	/* Make translation table */

#ifdef DEBUG_WLT
		printf("Tree level %d/%d, translation table:\n", i, tree_levels);		/* Print translation table */
		for(j=0; j<dict->len1; j++) printf("%c=%c ", dict->ia1[j], dic01[j]);
		printf("\n");
#endif
		/* In-place translation */
		int first_limit = (1<<(tree_levels-1-i)) - 1; 		// is the index of the dictionary
		int limit_stride = (1<<(tree_levels-i-1)) * 2;		// is the stride of the index of the dictionary
		int level_limits = (dict->len1 / limit_stride) + 1;	// is the number of strides before stepping out of boundary of dictionary
#ifdef DEBUG_WLT
		printf("This level has %d limits. The first [%c]. Stride: %d All limits:", level_limits, dict->ia1[first_limit], limit_stride);
		for(j=0; j<level_limits; j++) {
			int c = (dict->ia1[first_limit + limit_stride*j]);
			if(c<32) c=' ';
			printf("[%c] ", c);
		}
		printf("\n");
#endif
		k=0;
		for(j=0; j</*level_limits*/ (1<<i); j++) { /* sui 12-01-11: This bug was fixed. Do not use "level_limits" variable. use 1<<i. */
			int lmtidx = first_limit + limit_stride * j;
			if(lmtidx >= dict->len1) lmtidx = dict->len1-1;
			int le = dict->ia1[lmtidx];
			int zerocnt = wavelettree_inplace_transform(array1, bits, curr_limits[j*2], curr_limits[j*2+1], le);
			next_limits[k] = curr_limits[j*2]; k=k+1;
			next_limits[k] = curr_limits[j*2] + zerocnt; k=k+1;
			next_limits[k] = curr_limits[j*2] + zerocnt; k=k+1;
			next_limits[k] = curr_limits[j*2+1]; k=k+1;
		}
#ifdef DEBUG_WLT
		printf(" >> next_limits:\n");
		for(k=0; k<(level_limits)*4; k++) printf("[%d]=%d ", k, next_limits[k]);
		printf(" \n");
#endif
		curr_cr = new Crankselect();
		std::stringstream ssbitvec;  /* Copied from testrankselect.cpp */
		string bitvec;
		for(j=0; j<len; j++) ssbitvec<<(int)(bits[j]);
		bitvec = ssbitvec.str();
		curr_cr->build(bitvec);
		ret->rss[i]=curr_cr;

#ifdef DEBUG
		for(j=0; j<len; j++) {
			printf("%d", bits[j]);
		}printf("\n");
		
		printf(">> Translated string: \n");
		for(j=0; j<len; j++) {
			printf("%c", array1[j]);
		}printf("\n");
#endif

		int* tmp = next_limits;
		next_limits = curr_limits;
		curr_limits = tmp;
	}

#ifdef DEBUG
	printf("Encoding: \n");
	for(i=0; i<dict->len1; i++) {
		printf("%c ", (char)(dict->ia1[i]));
		for(j=0; j<tree_levels; j++) printf("%d", (i>>(tree_levels-j-1))&1);
		printf("\n");
	}
#endif

	printf("Dict size: %d, Tree level: %d\n", dict->len1, tree_levels);
	printf("wavelettree_build okay.\n");
	free(bits);
	free(next_limits);
	free(curr_limits);
	free(array1);
	free(dic01);
	return ret;
}

/* Input: a wavelet tree, the positions the search runs to, and the string to be searched.
 * Literal meaning: suppose we have a wltree for this string:
 * 
 *  000000000011111111112222222222333333333344444
 *  012345678901234567890123456789012345678901234   <-- Byte number
 * "rdkadrfs#  eiieepklptppPoppPccck   ip  eeeere"  <-- string content
 * 
 * wavelettree_rank(wltree, 40, 'e') will count "how many e's are there in range [0, 40], including 0 and 40".
 * Which in this case is 5.
 * 
 * This is quicker than just scanning the original string, especially when the original
 * string is very large. When it's 20M bytes, using a wavelet tree can save ~2/3 time.
 */
int wavelettree_rank(struct wavelettree* wltree, int until, int needle) {
#ifdef DEBUG_WLT
	printf(" >> wavelettree_rank(%d, %c)\n", until, needle);
#endif
	struct intarray2* dictc = wltree->dictc;
	int idx = -1; idx = index_of(dictc->ia1, dictc->len1, needle, 0);
	int i;
	int ret = -1;
	if(idx == -1) 
	{ assert(0); return -1; }
	char currbit = 0;
	
	int rk = 0; // Rank of this layer.
	int curr_right = until;
	int curr_zeros = 0, curr_ones = 0;
	int level_left = 0;
	int level_right = wltree->width-1;
	int level_zeros = 0, level_ones = 0;
	int left_ones = 0, left_zeros = 0;
	for(i=0; i<wltree->level; i++) {
		currbit = (idx>>(wltree->level-i-1))&1;
#ifdef DEBUG_WLT
		printf("Now counting bit #%d=%d in range [%d,%d], ", i, currbit, level_left, curr_right);
		printf("Subtree level %d is [%d,%d], curr_right=%d ", i, level_left, level_right, curr_right);
#endif
		level_zeros = level_right + 1 - wltree->rss[i]->rank(level_right);
		if(level_left > 0) level_zeros = level_zeros - (level_left-wltree->rss[i]->rank(level_left-1));
		level_ones = level_right - level_left + 1 - level_zeros;
#ifdef DEBUG_WLT
		printf("level_0s=%d, level_1s=%d ", level_zeros, level_ones);
#endif
		if(level_left > 0) {
			left_ones = wltree->rss[i]->rank(level_left-1);
			left_zeros = level_left - left_ones;
		}
#ifdef DEBUG_WLT
		printf("left_0s=%d, left_1s=%d ", left_zeros, left_ones);
#endif
		curr_ones = wltree->rss[i]->rank(curr_right);
		curr_zeros = curr_right + 1 - curr_ones;
#ifdef DEBUG_WLT
		printf("curr_0s=%d, curr_1s=%d\n", curr_zeros, curr_ones);
#endif
		if(i==wltree->level-1) break;
		if(currbit == 1) {
			level_left = level_left + level_zeros;
			curr_right = curr_ones - left_ones + level_left - 1;
		} else {
			level_right = level_right - level_ones;
			curr_right = curr_zeros - left_zeros + level_left - 1;
		}
	}
	i=wltree->level-1;
	curr_ones = wltree->rss[i]->rank(curr_right);
	curr_zeros = curr_right+1-curr_ones;
	curr_ones = curr_ones - left_ones;
	curr_zeros = curr_zeros - left_zeros;
	
	if(currbit==0) ret = curr_zeros;
	else ret = curr_ones;
#ifdef DEBUG_WLT
	printf("rank(%d, %c)=%d\n", until, (char)needle, ret);
#endif
	return ret;
}

/* Just prints a wavelet tree. */
void wavelettree_print(struct wavelettree* wltree){
	int i, j, alloc=0;
	printf("Wavelet tree has %d levels. It is %d bits wide.\n", wltree->level, wltree->width);
	for(i=0; i<wltree->level; i++) {
			if(wltree->width < 101) {
				printf("Tree @ level %d: ", i);
				for(j=0; j<wltree->width; j++) printf("%d", wltree->rss[i]->getBit(j));
				printf("\n");
			}
			alloc = alloc + wltree->rss[i]->getAllocSize();
	}
	printf("Total allocated space = %d bytes.\n", alloc);
}
