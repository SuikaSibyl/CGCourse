#include <iostream>
#include <stdio.h>
#include "course/AssignmentIndex.h"

#define ASSIGNMENT 1
int main(int argc, char* argv[])
{
#if(ASSIGNMENT==0)
	Assignment::Assignment0Main(argc, argv);
#endif
#if(ASSIGNMENT==1)
	Assignment::Assignment1Main(argc, argv);
#endif
}