#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alloc.h"

int main()
{
	//mmap to get page
	if(init_alloc())
		return 1;	//mmap failed

	char *str = alloc(512);
	char *str2 = alloc(512);

	if(str == NULL || str2 == NULL)
	{
		printf("alloc failed\n");
		return 1;
	}

	strcpy(str, "Hello, world!");
	if(strcmp("Hello, world!", str))
	  printf("Hello, world! test failed\n");
	
	else
	  printf("Hello, world! test passed\n");
	
	dealloc(str);
	dealloc(str2);

	printf("Elementary tests passed\n");
	
	printf("Starting comprehensive tests (see details in code)\n");

	/*
	Comprehensive tests:
	1. Populate entire thing in blocks of for loop (a's, b's, c's, and d's) equal size.
	2. Dealloc c's, reallocate and replace with x's.
	3. 
	*/

	/*** test 1 ****/
	
	//Generating ideal strings for comparison
	char stringA[1024], stringB[1024], stringC[1024], stringD[1024], stringX[1024];
	for(int i = 0; i < 1023; i++)
	{
		stringA[i] = 'a';
		stringB[i] = 'b';
		stringC[i] = 'c';
		stringD[i] = 'd';
		stringX[i] = 'x';
	}

	stringA[1023] = stringB[1023] = stringC[1023] = stringD[1023] = stringX[1023] = '\0';

	char *strA = alloc(1024);
	char *strB = alloc(1024);
	char *strC = alloc(1024);
	char *strD = alloc(1024);

	for(int i = 0; i < 1023; i++)
	{
		strA[i] = 'a';
		strB[i] = 'b';
		strC[i] = 'c';
		strD[i] = 'd';
	}
	strA[1023] = strB[1023] = strC[1023] = strD[1023] = '\0';

	if(strcmp(stringA, strA) == 0 && strcmp(stringB, strB) == 0 && strcmp(stringC, strC) == 0 && strcmp(stringD, strD) == 0)
	  printf("Test 1 passed: allocated 4 chunks of 1KB each\n");
	else
	  printf("Test 1 failed: A: %d, B: %d, C: %d, D: %d\n", strcmp(stringA, strA), strcmp(stringB, strB), strcmp(stringC, strC), strcmp(stringD, strD));

	/**** test 2 ****/
	
	dealloc(strC);

	char *strX = alloc(1024);
	for(int i = 0; i < 1023; i++)
	{
		strX[i] = 'x';
	}
	strX[1023] = '\0';

	if(strcmp(stringX, strX) == 0)
	  printf("Test 2 passed: dealloc and realloc worked\n");
	else
	  printf("Test 2 failed: X: %d\n", strcmp(stringX, strX));

	/*** test 3 ***/
	
	char stringY[512], stringZ[512];
	for(int i = 0; i < 511; i++)
	{
		stringY[i] = 'y';
		stringZ[i] = 'z';
	}
	stringY[511] = stringZ[511] = '\0';

	dealloc(strB);

	char *strY = alloc(512);
	char *strZ = alloc(512);
	
	for(int i = 0; i < 511; i++)
	{
		strY[i] = 'y';
		strZ[i] = 'z';
	}
	strY[511] = strZ[511] = '\0';

	if(strcmp(stringY, strY) == 0 && strcmp(stringZ, strZ) == 0)
	  printf("Test 3 passed: dealloc and smaller realloc worked\n");
	else
	  printf("Test 3 failed: Y: %d, Z: %d\n", strcmp(stringY, strY), strcmp(stringZ, strZ));


	// merge checks
	//test 4: free 2x512, allocate 1024
	
	dealloc(strZ);
	dealloc(strY);
	strY=alloc(1024);
	for(int i = 0; i < 1023; i++)
	{
		strY[i] = 'x';
	}
	strY[1023] = '\0';

	if(strcmp(stringX, strY) == 0)
	  printf("Test 4 passed: merge worked\n");
	else
	  printf("Test 4 failed: X: %d\n", strcmp(stringX, strX));

	//test 5: free 2x1024, allocate 2048
	
	dealloc(strX);
	dealloc(strY);
	strX= alloc(2048);
	char  stringM[2048];
	for (int i=0;i<2047;i++){
		strX[i]=stringM[i]='z';
	}
	strX[2047]=stringM[2047]='\0';
	if (!strcmp(stringM, strX))
		printf("Test 5 passed: merge alloc 2048 worked\n");
	else
		printf("Test5 failed\n");
	///////////////////////////
	// Test 6: Checks if allocator correctly reuses a previously freed 128-byte block.
	char *ptr1 = alloc(64);
	char *ptr2 = alloc(128);
	char *ptr3 = alloc(256);

	dealloc(ptr2);

	char *ptr4 = alloc(128);

	if (ptr4 == ptr2)
		printf("Test 6 passed: Successfully reused freed 128-byte block\n");
	else
		printf("Test 6 failed: Did not reuse the expected freed block\n");

	dealloc(ptr1);
	dealloc(ptr3);
	dealloc(ptr4);

	// Test 7: Verifies that two adjacent freed blocks merge correctly into a larger block.
	char *a = alloc(1024);
	char *b = alloc(1024);
	char *c = alloc(2048);

	dealloc(a);
	dealloc(b);

	char *d = alloc(2048);

	if (d == a)
		printf("Test 7 passed: Successfully merged two 1024-byte blocks into one 2048-byte block\n");
	else
		printf("Test 7 failed: Block merging did not work as expected\n");

	dealloc(c);
	dealloc(d);

	// Test 8: Checks if allocator correctly handles alternating allocation and deallocation.
	char *alt1 = alloc(256);
	char *alt2 = alloc(512);
	char *alt3 = alloc(256);

	dealloc(alt2);

	char *alt4 = alloc(512);

	if (alt4 == alt2)
		printf("Test 8 passed: Successfully reused previously freed 512-byte block\n");
	else
		printf("Test 8 failed: Did not reuse previously freed block\n");

	dealloc(alt1);
	dealloc(alt3);
	dealloc(alt4);

	if (cleanup())
		return 1;

	return 0;
}
