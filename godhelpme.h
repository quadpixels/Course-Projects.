#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "rs/rswrapper.h"


struct int2 {
	int i1;
	int i2;
};

struct intarray2 {
	int* ia1;
	int len1;
	int* ia2;
	int len2;
};

struct intarray {
	int* ia;
	int len;
};

/* Wavelet Tree, CS 11-27-11 */
struct wavelettree {
	struct intarray2* dictc;
	int level;
	int width; /* 一层的宽度 */
	Crankselect** rss;
};
struct wavelettree* wavelettree_build(int* array, int len, struct intarray2* dict);
void wavelettree_print(struct wavelettree* wltree);
int wavelettree_rank(struct wavelettree* wltree, int until, int needle);

int val(int a);
int* alphabetical_sort(int* array, int len);
int findinarray_ff(int* array, int arraylen, int fr, int lr, int needle, struct int2* result);
int get_dicandc(int* alpha, int len, struct intarray2* result);
int find_num_of_occurances(int*, int*, struct wavelettree*, int, struct intarray2*, int*, int, struct int2*);
int* occurances_to_locations(int* sa, struct int2* si2, int arraylen);
