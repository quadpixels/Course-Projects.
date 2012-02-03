/* Sui Chen's CSC 7300 Project
 * Prof Rahul Shah
 * 11-28-2011
 * 
 * This algorithm is the Skewed sort algorithm in Juha Karkkainen and
 * Peter Sanders' paper, Simple Linear Work Suffix Array Construction.
 * Just implemented the idea. I did not copy the code they attached in
 * the end of their paper b/c it was toooooooooo hard to understand and
 * it was extreeeemely optimized. Also in order to practice the survival
 * skill I had to implement this from scratch.
 * 
 * But what's different is in my implementation the sort012() returns a
 * lookup array instead of a suffix array. Needs an additional effort to
 * convert a lookup array into a suffix array with get_SA().
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sort012.h"

/* Change this macro definition to make the program show debug messages */
//#define DEBUG

#define BUCKET_SIZE (10000)

int* g_array;
int g_reclevel = 0;

//int g_array[]={'m', 'i', 's', 's', 'i', 's', 's', 'i', 'p', 'p', 'i'};
//int g_array[]={'3', '3', '2', '1', '5', '5', '4'};

void print_rec() { int i; for(i=0; i<g_reclevel; i++)printf(">%d\t", g_reclevel); }

int array_get(int* array, int size, int pos) {
	assert(pos>=0);
	if(pos >= size) return 0;
	else return array[pos];
}

//			input array | input array size | output array rank
int* sort_12(int* ia, int isz) {
	int i,j;
	int* oar = (int*)malloc(sizeof(int)*isz); /* output array rank */
	int n0, n1, n2; /* Number of suff1's and suff2's */
	n0=n1=n2=(int)(isz/3);
	if(isz%3==1) {n0++;}
	else if(isz%3==2) {n0++; n1++;}
	
	int* lookup = (int*)malloc(sizeof(int)*isz); /* lookup: from subscript number to its rank */
	//memset(lookup, 0, sizeof(int)*isz);
	for(i=0; i<isz; i++) lookup[i]=0;
#ifdef DEBUG
	print_rec(); printf("sort_12()\n"); 
	print_rec(); for(i=0; i<isz; i++)printf("%d ", ia[i]); printf("\n");
#endif
	int* a=(int*)malloc(sizeof(int)*(n1+n2)); /* array of subscripts, for sorting */
	int* a_temp  =(int*)malloc(sizeof(int)*(n1+n2));
	int* a_sorted=(int*)malloc(sizeof(int)*(n1+n2));
	
	//PITFALL HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for(i=0; i<n1; i++) a[i]=i*3+1;
	for(i=0; i<n2; i++) a[i+n1]=i*3+2;
	
	//for(i=0; i<n1; i++) a[i*2]=i*3+1;
	//for(i=0; i<n2; i++) a[i*2+1]=i*3+2;
	
	memcpy(a_temp, a, sizeof(int)*(n1+n2));
	
	/* Sort suff1's and suff2's */
//	int ctr[BUCKET_SIZE]; /* counter */
	int ctr_sz=isz+BUCKET_SIZE; //May waste memory space, I added this to avoid segmentation fault
	int* ctr = (int*)malloc(sizeof(int)*ctr_sz); for(i=0;i<ctr_sz;i++)ctr[i]=0;
	for(j=2; j>=0; j--) {
		for(i=0; i</*BUCKET_SIZE*/ctr_sz; i++) ctr[i]=0;
		for(i=0; i<n1+n2; i++) {
			ctr[array_get(ia, isz, a_temp[i]+j)]++; 
			}
		for(i=0; i</*BUCKET_SIZE-1*/ctr_sz-1; i++) ctr[i+1]+=ctr[i];
		for(i=n1+n2-1; i>=0; i--) {
			int p=array_get(ia, isz, a_temp[i]+j);
		
		a_sorted[ctr[p]-1]=a_temp[i];
			ctr[p]--;
		}
		
		if(j!=0) {
			int* tmp = a_sorted; a_sorted=a_temp; a_temp=tmp;
		}
	}
	
	/* Rank suff1's and suff2's */
	int rk=0, isAllUnique=1;
	for(i=0; i<n1+n2; i++){
		lookup[a_sorted[i]]=rk+1;
		if(i!=n1+n2-1) {
			if((array_get(ia, isz, a_sorted[i])!=array_get(ia, isz, a_sorted[i+1]))
			|| (array_get(ia, isz, a_sorted[i]+1)!=array_get(ia, isz, a_sorted[i+1]+1))
			|| (array_get(ia, isz, a_sorted[i]+2)!=array_get(ia, isz, a_sorted[i+1]+2))
			)
			rk=rk+1;
			else isAllUnique=0;
		}
	}
#ifdef DEBUG
	print_rec(); printf("rank: \n");
	print_rec(); for(i=0; i<isz; i++)printf("%d", lookup[i]); printf("\n");
#endif
	int isdummy = n0-n1; /* */
	int* rc = (int*)malloc(sizeof(int)*(n1+n2+isdummy));/* rc = recursion */
	memset(rc, 0, sizeof(int)*(n1+n2));
	
	if(isAllUnique==0) {
#ifdef DEBUG
		print_rec(); printf("Need recursion.\n");
#endif
		int j=0;
		//for(i=0; i<n1+n2; i++) {rc[i]=lookup[a[i]]+1;}
		for(i=0; i<n1; i++) {rc[j]=lookup[a[i]]+1; j++;}
		if(isdummy==1) {rc[j]=1; j++;}
		for(i=0; i<n2; i++) {rc[j]=lookup[a[i+n1]]+1; j++;}
		
		g_reclevel++;
		int* rank1 = sort_012(rc, (n1+n2+isdummy));
		g_reclevel--;
#ifdef DEBUG
		print_rec(); printf("## a[1] rank1[i] lookup[a[i]]\n");
#endif
		for(i=0; i<isz; i++)lookup[i]=0;
		j=0;
		for(i=0; i<n1+n2; i++) {
			if(isdummy==1 && i==n1) {assert(rc[j]==1); j=j+1;}
//			assert(a[i]<isz && a[i]>0);
			lookup[a[i]]=rank1[j];
			j=j+1;
		}
		free(rank1);
		/* lookup has changed now */
	}
#ifdef DEBUG
	print_rec(); printf("No need to recursion.\n");
//	print_rec(); printf("sort_12() returns lkp:\n");
//	print_rec(); printf("  subsc -> rank  \n");
//	for(i=0; i<n1+n2; i++) {print_rec(); printf("   %d  ->  %d\n", a[i], lookup[a[i]]);}
	print_rec();printf("sort_12() ends.\n");
#endif
	free(ctr);
	free(rc);
	free(oar);
	free(a);
	free(a_temp);
	free(a_sorted); 
	
	return lookup;
}

int* sort_012(int* ia, int isz) {
	int n0, n1, n2, i, j, ctr_sz;
	ctr_sz = BUCKET_SIZE + isz;
	int* ctr=(int*)malloc(sizeof(int)*ctr_sz);
	for(i=0;i<ctr_sz;i++)ctr[i]=0;
//	int ctr[BUCKET_SIZE]; //culprit?
//	for(i=0; i<BUCKET_SIZE; i++) ctr[i]=0;
	
#ifdef DEBUG
	print_rec(); printf("sort_012() recursion:%d isz:%d\n", g_reclevel, isz);
	print_rec(); for(i=0; i<isz; i++) {printf("%d ", ia[i]);} printf("\n");
#endif
	int* lookup = sort_12(ia, isz);
	n0 = n1 = n2 = (int)((isz-(isz%3))/3);
	if(isz%3==1) n0++;
	if(isz%3==2) {n0++; n1++;}
	
	/* merge 0 and 1 */
	int* a01 = (int*)malloc(sizeof(int)*(n0+n1)); /* array index */
	int* a01_sorted = (int*)malloc(sizeof(int)*(n0+n1));
	memset(a01, 0, sizeof(int)*(n0+n1));
	memset(a01_sorted, 0, sizeof(int)*(n0+n1));
	for(i=0; i<n0; i++) a01[i]=i*3;
	for(i=0; i<n1; i++) a01[i+n0]=i*3+1;
		/* digit 2, which we have to look @ the lookup table. */
		for(i=0; i<n0+n1; i++) {
			int l;
			if((a01[i]+1) < isz) {l = lookup[a01[i]+1]; }
			else l=0;
			ctr[l]++;
		}
		for(i=1; i<ctr_sz; i++) {ctr[i]=ctr[i]+ctr[i-1];}
		for(i=n0+n1-1; i>=0; i--) {
			int l;
			if((a01[i]+1) < isz) {l = lookup[a01[i]+1]; }
			else l=0;
			a01_sorted[ctr[l]-1]=a01[i];
			ctr[l]--; }

		/* digit 1 */
		int* tmp=a01_sorted; a01_sorted=a01; a01=tmp;
		for(i=0; i<ctr_sz; i++) ctr[i]=0;
		for(i=0; i<n0+n1; i++) {ctr[array_get(ia, isz, a01[i])]++; }
		for(i=1; i<ctr_sz; i++) ctr[i]=ctr[i]+ctr[i-1];
		for(i=n0+n1-1; i>=0; i--) {
			int l = array_get(ia, isz, a01[i]);
			a01_sorted[ctr[l]-1]=a01[i]; ctr[l]--;
		}
#ifdef DEBUG
	printf("a01_sorted: "); for(i=0; i<n0+n1; i++)printf("%d ", a01_sorted[i]); printf("\n");
#endif
	
	/* merge 0 and 2 */
	int* a02 = (int*)malloc(sizeof(int)*(n0+n2));
	int* a02_sorted=(int*)malloc(sizeof(int)*(n0+n2));
	for(i=0; i<n0; i++) a02[i]=i*3; for(i=0; i<n2; i++) a02[i+n0]=i*3+2;
	
		/* digit 3, which we have to look @ the lookup table. */
		for(i=0; i<ctr_sz; i++) ctr[i]=0;
		for(i=0; i<n0+n2; i++) {
			int l; if(a02[i]+2 < isz) l=lookup[a02[i]+2]; else l=0;
			ctr[l]++; }
		for(i=0; i<ctr_sz-1; i++) ctr[i+1]=ctr[i+1]+ctr[i];
		for(i=n0+n2-1; i>=0; i--) {
			int l; if(a02[i]+2 < isz) l=lookup[a02[i]+2]; else l=0;
			a02_sorted[ctr[l]-1]=a02[i];
			ctr[l]--; }
			
		/* digit 2 and 1 */
		for(j=1; j>=0; j--) {
			int* tmp=a02_sorted; a02_sorted=a02; a02=tmp;
			for(i=0; i<ctr_sz; i++) ctr[i]=0;
			for(i=0; i<n0+n2; i++) ctr[array_get(ia, isz, a02[i]+j)]++;
			for(i=1; i<ctr_sz; i++) ctr[i]=ctr[i]+ctr[i-1];
			for(i=n0+n2-1; i>=0; i--) {
				int l=array_get(ia, isz, a02[i]+j);
				a02_sorted[ctr[l]-1]=a02[i]; ctr[l]--;
			}
		}
#ifdef DEBUG
	print_rec(); printf("a02_sorted: "); for(i=0; i<n0+n2; i++)printf("%d ", a02_sorted[i]); printf("\n");
	print_rec(); printf("mod 3 equals 0's:\n ## a01: \n"); print_rec();
	for(i=0; i<n0+n1; i++) {
		int p=a01_sorted[i];
		if(p%3==0){ printf("%d ", p);}
	}
	printf("\n ## a02: \n");print_rec();
	for(i=0; i<n0+n2; i++) {
		int p=a02_sorted[i];
		if(p%3==0){ printf("%d ", p);}
	}
	printf("\n");
#endif
	
	/* merge 01 and 02 */
	int p1=0, p2=0, p012=0;
	int* a012 = (int*)malloc(sizeof(int)*isz); /* isz=n0+n1+n2 */
	for(i=0; i<isz; i++)a012[i]=0;
//	printf("n0+n1=%d n0+n2=%d\n", (n0+n1), (n0+n2));
	while(!((p1==n0+n1) && (p2==n0+n2))) {
		if(p1==n0+n1 && p2!=n0+n2) {
			a012[p012]=a02_sorted[p2]; p012++; p2++;
		}
		else if((p2==n0+n2) && (p1!=n0+n1)) {
			a012[p012]=a01_sorted[p1]; p012++; p1++;
		}
		else if((a01_sorted[p1]%3==0)&&(a02_sorted[p2]%3==0)) 
		{
			assert((a01_sorted[p1]==a02_sorted[p2])); a012[p012]=a01_sorted[p1]; p012++; p1++; p2++;
		}
		else if(a01_sorted[p1]%3==0) {
			assert(a02_sorted[p2]%3!=0);
			a012[p012]=a02_sorted[p2]; p012++; p2++;
		}
		else if(a02_sorted[p2]%3==0) {
			assert(a01_sorted[p1]%3!=0);
			a012[p012]=a01_sorted[p1]; p012++; p1++; 
		}
		else {
			int lkpp1 = lookup[a01_sorted[p1]];
			int lkpp2 = lookup[a02_sorted[p2]];
			assert(lkpp1!=lkpp2);
			if(lkpp1 > lkpp2)
				{ a012[p012]=a02_sorted[p2]; p012++; p2++; }
			else { a012[p012]=a01_sorted[p1]; p012++; p1++; }
		}
	}
	
	int* ret_lookup = (int*)malloc(sizeof(int)*isz);
	for(i=0; i<isz; i++) ret_lookup[a012[i]]=i+1;

#ifdef DEBUG	
	print_rec(); printf("a012_merged: \n"); 
	for(i=0; i<isz; i++)printf("%d ", a012[i]);
	printf("\n");
	for(i=0; i<isz; i++) {printf("%d ", ret_lookup[i]);}
	printf("\n");
	print_rec(); printf("sort_012() ends.\n");
#endif
	free(ctr);
	free(lookup);
	free(a01);
	free(a02);
	free(a01_sorted);
	free(a02_sorted);
	free(a012); // What the heck
	
	return ret_lookup;
}
/*
int main(void) {
	char in[10000];
	scanf("%s", in);
	int len = strlen(in);
	g_array=(int*)malloc(sizeof(int)*len); int i; for(i=0; i<len; i++) g_array[i]=(int)in[i];
	printf("Strlen: %d.\n", len);
	int* g_rank = 0;
	g_rank = sort_012(g_array, len);
	printf(" Thank you!\n");
	
	free(g_array);
	free(g_rank);
	
	printf(" Freed memory.\n");
	
	return 0;
}*/

/* Input: the lookup array and its length */
int* get_SA(int* lkp, int len) {
	int i;
	int* sa = (int*)malloc(sizeof(int)*len);
	for(i=0; i<len; i++)sa[i]=0;
	for(i=0; i<len; i++)sa[lkp[i]-1]=i; /* Lookup says suff @ offset n is
		the K'th, starts from 1, so we subtract it by 1 here */ 
	return sa;
}

/* Input:  the array, its suffix array and the length of the string 
   Output: the BWTed array 
*/
/* array */int* get_BWT(int* array, int* sa, int len) {
	int* bwt = (int*)malloc(len*sizeof(int));
	int i; for(i=0; i<len; i++) bwt[i]=array[(len+sa[i]-1)%len];
	return bwt;
}

/* eof */
