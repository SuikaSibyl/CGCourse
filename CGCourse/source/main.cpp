#include <iostream>
#include <stdio.h>
#include "course/AssignmentIndex.h"
#include "Settings.h"

int main(int argc, char* argv[])
{
#if(ASSIGNMENT==0)
	Assignment::Assignment0Main(argc, argv);
#endif
#if(ASSIGNMENT==1)
	Assignment::Assignment1Main(argc, argv);
#endif
#if(ASSIGNMENT==2)
	Assignment::Assignment2Main(argc, argv);
#endif
#if(ASSIGNMENT==3)
	Assignment::Assignment3Main(argc, argv);
#endif
}