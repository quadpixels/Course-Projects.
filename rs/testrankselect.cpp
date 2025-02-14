#include <iostream>
#include <sstream>
#include <math.h>
#include "rswrapper.h"

#define bvlength 45
#define MY_DEBUG

int main (int argc, char * const argv[]) {
  
	Crankselect rstest;	
	std::stringstream ssbitvec;
	string bitvec;
	
	//Generating random bit vector
	srand ((unsigned)time(NULL));
	for(unsigned int i=0; i<bvlength; i++)
		ssbitvec << rand()%2;
	
	
	
	/* cs 1119 */
	#ifdef MY_DEBUG
	int ptr = ssbitvec.tellg();
	char ch=' ';
	printf("String:");
	while(ch=='0' || ch=='1' || ch==' ') {
		printf("%s", &ch);
		ch=ssbitvec.get();
	}
	printf("\n");
	#endif
	
	bitvec = ssbitvec.str();

	//Building rank-select structure
	rstest.build(bitvec);
	
	//Testing getbit
	for(unsigned int i=0; i<bvlength; i++)
		if((bitvec[i] == 1 && !rstest.getBit(i)) || (bitvec[i] == 0 && rstest.getBit(i))) cout << "error!\n";	
	
	//Testing rank and select
	unsigned int count1=0;
	for(unsigned int i=0; i<bvlength; i++)
	{
		if(bitvec[i] == '1') 
		{
			count1++;
			if(rstest.select(count1) != i) cout << "errror!\n";
		}
		
		if(rstest.rank(i) != count1) cout << "error!\n";
	}
	
    cout << "Allocates sapce:" << rstest.getAllocSize() << " bytes\n";
    return 0;
}
