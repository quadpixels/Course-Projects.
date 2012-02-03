/* Sui Chen CSC7300
 * Dr. Rahul Shah   
 * 11-21-11
 * 12-01-11 Found the bug in the wavelet tree. Fixed.
 * 12-02-11 Made the interface a little more beautiful.
 * Test cases and entry point to the program.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sort012.h"
#include "godhelpme.h"

/* For Benchmark use. */
#include <sys/time.h>
#include <unistd.h>
struct timeval tv_start, tv_sa, tv_end;

#define DEBUG_TEST
#define FILEREAD_LINEWIDTH 1000000

int* g_testarray; // the haystack file
int g_len;    // the haystack file length
int* g_rank;  // the output rank, or SA, you name it

int* readfile_haystack(char* filename, int* len) {
	int sz, sz_read, i;
	int* ret;
	FILE* file = fopen(filename, "r");
	if(file==NULL) {
		printf("File open error, maybe the file does not exist.\nPlease enter another file name.\n");
		return (int*)NULL;
	}
	
	fseek(file, 0L, SEEK_END);
	sz=ftell(file);
	printf("File %s is %d bytes long.\n", filename, sz);
	char* ch = (char*)malloc(sz*sizeof(char));
	fseek(file, 0L, SEEK_SET);
	sz_read=fread(ch, sizeof(char), sz, file);
	assert(sz_read==sz);
	
	ret=(int*)malloc(sizeof(int)*sz);
	*len = sz;
	for(i=0; i<sz; i++)ret[i]=ch[i];
	
	free(ch);
	fclose(file);
	return ret;
}

struct intarray** readfile_needles(char* filename, int* count) {
	int i, j;
	FILE* file = fopen(filename, "r");
	if(file==NULL) {
		printf("File open unsuccessful. Proceeding with no pattern file name.\n"); return NULL;
	}
	
	fscanf(file, "%d\n", count);
	printf("Needle count: %d\n", *count);
	struct intarray** ret = (struct intarray**)malloc(sizeof(struct intarray*)*(*count));
	
	for(i=0;i<*count;i++) {
		char tmp[FILEREAD_LINEWIDTH];
		char tmp1[2];
//		j = fscanf(file, "%[^\n]s\n", tmp); /* Remove the whitespace in the back*/
		j = fscanf(file, "%s\n", tmp);
		int len = strlen(tmp);
//		fscanf(file, "[\n\r]", tmp1);
		if(len < 100) {
			printf("%d. %s(%d)\n", i, tmp, len);
		}else{printf("%d. (a long needle, %d bytes)\n", i, len);}
		int* currnode = (int*)malloc(sizeof(int)*len);
		for(j=0;j<len;j++) currnode[j]=tmp[j];
		ret[i]=(struct intarray*)malloc(sizeof(struct intarray));
		ret[i]->ia=currnode;
		ret[i]->len=len;
	}
	return ret;
}

void print_suffix(int offset) {
	int omit=15; //if exceed 15 bytes then omit
	int i; for(i=offset; i<g_len&&i<offset+omit; i++) {
		printf("%c", g_testarray[i]);
	}
	if(i==offset+omit)printf(" ... ");
}

void unit_test1() {
		
#ifdef DEBUG_TEST
	char test[]="mississippi";
	int test_len=strlen(test);
	int* test_array=(int*)malloc(sizeof(int)*test_len);
	int i;for(i=0;i<test_len;i++)test_array[i]=test[i];
	int* test_lkp=sort_012(test_array, test_len);
	int* test_sa =get_SA(test_lkp, test_len);
	int* test_bwt=get_BWT(test_array, test_sa, test_len);
	int* test_alpha=alphabetical_sort(test_array, test_len);
	
	printf("\nThe lookup array:\n");
	for(i=0;i<test_len;i++) {
		printf("%d ", i); print_suffix(i);
		printf(" -> %d\n", test_lkp[i]);
	}
	printf("\nThe suffix array:\n");
	for(i=0;i<test_len;i++) {
		printf("SA[%d]=\t%d   ", i, test_sa[i]);
		print_suffix(test_sa[i]);
		printf("\n");
	}
	printf("\nThe BWT'ed string:\n");
	for(i=0;i<test_len;i++)printf("%c", test_bwt[i]);
	printf("\n");
	
	printf("\nThe alphabetically sorted string: \n");
	for(i=0;i<test_len;i++)printf("%c", test_alpha[i]);
	printf("\n");
	
	struct int2 fl;
	int result1=findinarray_ff(test_alpha, test_len, 0, 10, 's', &fl);
	printf("(%d)s in sorted array from 0 to 11, first is %d last is %d\n",
		result1, fl.i1, fl.i2);
	
	struct intarray2 dictc; /* dict and c */
	get_dicandc(test_alpha, test_len, &dictc);
	printf("dict size=%d\n", dictc.len1);
	for(i=0; i<dictc.len1; i++) printf("%c %d\n", dictc.ia1[i], dictc.ia2[i]);
	
	struct wavelettree* test_bwt_wlt = wavelettree_build(test_bwt, test_len, &dictc);
	
	printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
	for(i=0; i<test_len; i++) printf("%c", test_bwt[i]); printf("\n");
	wavelettree_print(test_bwt_wlt);
	wavelettree_rank(test_bwt_wlt, 3, 's');
	wavelettree_rank(test_bwt_wlt, 4, 'p');
	wavelettree_rank(test_bwt_wlt, test_len-1, 'i');
	printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
	
	int needle1[]={'s', 's', 'i'};
	int needle1len = 3;
	find_num_of_occurances(test_alpha, test_bwt, test_bwt_wlt, test_len, &dictc, needle1, needle1len, &fl);
	
	printf("\n");
	int needle2[]={'i'};
	int needle2len = 1;
	find_num_of_occurances(test_alpha, test_bwt, test_bwt_wlt, test_len, &dictc, needle2, needle2len, &fl);
	
	printf("\n");
	int needle3[]={'p', 'i'};
	int needle3len = 2;
	find_num_of_occurances(test_alpha, test_bwt, test_bwt_wlt, test_len, &dictc, needle3, needle3len, &fl);
	
	printf("\n");
	int needle4[]={'m', 'i', 's', 's', 'i', 's', 's', 'i', 'p', 'p', 'i'};
	int needle4len = 11;
	find_num_of_occurances(test_alpha, test_bwt, test_bwt_wlt, test_len, &dictc, needle4, needle4len, &fl);

	printf("\n");
	int needle5[]={'s'};
	int needle5len = 1;
	find_num_of_occurances(test_alpha, test_bwt, test_bwt_wlt, test_len, &dictc, needle5, needle5len, &fl);
	
	free(test_array); free(test_lkp); free(test_sa); free(test_alpha); free(test_bwt);
	printf(">> Unit test 1 over.\n");
	
	occurances_to_locations(test_sa, &fl, test_len);
#endif
}

void print_beautiful_occurances(int* array, 
	int array_len, struct intarray* needle, int* occ, int occ_count) 
{
	int i,j,k;
	for(i=0; i<occ_count; i++) {
		j=occ[i];
		assert(j<=(array_len-(needle->len))); assert((needle->len)>0);
		printf("No.%d [%d] (", i+1, j);
		
		k=0;
		while(k<(2+needle->len) && j+k<array_len) 
			{ 
				if(j+k>=0)
				printf("%c",array[j+k]);
				else printf(" ");
				k=k+1; 
			}
		if(j+k<array_len)
			printf(" ... ");
		printf(")\n");
	}
}

long time_delta(struct timeval* start, struct timeval* end) {
	long mtime, seconds, useconds;
	seconds = end->tv_sec - start->tv_sec;
	useconds = end->tv_usec - start->tv_usec;
	mtime = (seconds * 1000.0 + useconds/1000.0) + 0.5;
	return mtime;
}

int main(int argc, char** argv) {
/*	unit_test1();
	return 0;*/
	char isPrintPretty = 0;
	if(argc > 1 && strcmp(argv[1], "beauty")==0) {isPrintPretty=1; printf("Print occurances.\n");}
	char filename[1000]; int len;
	int i, j;
	char filename_needles[1000]; int len_needles;
	int* array = NULL;
	
	while(array==NULL) {
		printf(" >> Please input filename.\n >>");
		scanf("%s", filename);
		array = readfile_haystack(filename, &len);
	}
	
	printf(" >> Please input pattern filename. If you don't want a pattern file, enter random characters and press enter.\n >>");
	scanf("%s", filename_needles);
	struct intarray** needles = readfile_needles(filename_needles, &len_needles);
	
	// Profiling start
	gettimeofday(&tv_start, NULL);
	
	printf(" >> Building lookup array.\n");
	int* array_lkp = sort_012(array, len);
	printf(" >> Building suffix array.\n");
	int* array_sa  = get_SA(array_lkp, len);
	printf(" >> Building BWT.\n");
	int* array_bwt = get_BWT(array, array_sa, len);
	printf(" >> Building dictionary.\n");
	int* array_alpha = alphabetical_sort(array, len);
	
	
	struct intarray2 dictc;
	struct int2 fl;
	int nocc;
	get_dicandc(array_alpha, len, &dictc);
	
	
	
	printf(" >> Building wavelet tree.\n");
//	struct wavelettree* array_wavelettree = wavelettree_build(array, len, &dictc);
	struct wavelettree* array_bwt_wlt = wavelettree_build(array_bwt, len, &dictc);

	/* Completed generating SA, BWT, WLT */
	gettimeofday(&tv_sa, NULL);


	wavelettree_print(array_bwt_wlt);
	
#ifdef DEBUG	
	printf("dict size=%d\n", dictc.len1);
	for(i=0; i<dictc.len1; i++) printf("(%c, %d)\n", dictc.ia1[i], dictc.ia2[i]);
	printf("\n");
#endif
#ifdef DEBUG	
	printf(" >> Testing wavelet tree.\n");
	for(i=0; i<dictc.len1; i++) {
		int rk = wavelettree_rank(array_bwt_wlt, len-1, dictc.ia1[i]);
		int rk2 = (i==dictc.len1-1) ? len - dictc.ia2[i] : dictc.ia2[i+1]-dictc.ia2[i];
		printf("%d. [%c] tree_rk: %d dict_rk: %d\n",
				i,	dictc.ia1[i], rk, 		  rk2);
	}
#endif	

	for(i=0;i<len_needles;i++) {
		nocc = find_num_of_occurances(array_alpha, array_bwt, array_bwt_wlt, len, &dictc, needles[i]->ia, needles[i]->len, &fl);
		int* array_occ = occurances_to_locations(array_sa, &fl, len);
		/* Don't print if the input is massively large! */
		if(isPrintPretty==1)
			print_beautiful_occurances(array, len, needles[i], array_occ, nocc); 
		free(array_occ);
		
		printf("Pattern No.%d, [", i);
		for(j=0; j<needles[i]->len; j++) {
			printf("%c", needles[i]->ia[j]);
			if(j > 100) { printf(" ... "); break;}
		}
		printf("], ");
		
		if(nocc>0) printf("Found %d occurances.", nocc);
		else printf("Not found.");
		printf("\n");
	}
	
	// Profiling end
	gettimeofday(&tv_end, NULL);
	long mtime;
	mtime = time_delta(&tv_start, &tv_sa);
	printf("\n==========Running time!==========\n");
	printf("Building SA, BWT and alphabetically sorting used %ld milliseconds.\n", mtime);
	
	mtime = time_delta(&tv_sa, &tv_end);
	printf("Searching %d patterns used %ld milliseconds.\n", len_needles, mtime);
	
	
	printf("\nIf you want to search any other keywords, enter them here. (Ctrl+C to exit)\n");
	char keyin[1000];
	int ikeyin[1000];
	struct intarray iakeyin; iakeyin.ia=ikeyin; iakeyin.len=0;
	
	while(1) {
		printf(" >>");
		scanf("%s", keyin);
		if(strcmp(keyin, "~~~")==0) break;
		gettimeofday(&tv_start, NULL);
		iakeyin.len = strlen(keyin); for(i=0; i<iakeyin.len; i++)ikeyin[i]=keyin[i];
		nocc = find_num_of_occurances(array_alpha, array_bwt, array_bwt_wlt, len, &dictc, iakeyin.ia, iakeyin.len, &fl);
		int* array_occ = occurances_to_locations(array_sa, &fl, len);
		gettimeofday(&tv_sa, NULL);
		mtime = time_delta(&tv_start, &tv_sa);
		if(isPrintPretty==1) print_beautiful_occurances(array, len, &iakeyin, array_occ, nocc);
		free(array_occ);
		if(nocc!=-1) printf(" >> Found %d occurances in %ld milliseconds.\n", nocc, mtime);
		else printf(" >> Not found. Time used: %ld milliseconds.\n", mtime);
	}
	
	return 0;
}
