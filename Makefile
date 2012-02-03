sort012test: sort012.c sort012test.c
#	g++ -g -fmudflap -lmudflap sort012.c sort012test.c godhelpme.c rs/bitarray.cpp -o sort012
	g++ -pg sort012.c sort012test.c godhelpme.c rs/bitarray.cpp -o sort012
clean:
	rm sort012
