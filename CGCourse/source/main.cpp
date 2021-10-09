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
#if(ASSIGNMENT==4)
	Assignment::Assignment4Main(argc, argv);
#endif
#if(ASSIGNMENT==5)
	Assignment::Assignment5Main(argc, argv);
#endif
#if(ASSIGNMENT==6)
	Assignment::Assignment6Main(argc, argv);
#endif
#if(ASSIGNMENT==7)
	Assignment::Assignment7Main(argc, argv);
#endif
#if(ASSIGNMENT==8)
	if (std::string(argv[1]) == "curve_editor")
		Assignment::Assignment8Main(argc, argv);
	else if (std::string(argv[1]) == "raytracer")
		Assignment::Assignment7Main(argc, argv);
#endif
#if(ASSIGNMENT==9)
	Assignment::Assignment9Main(argc, argv);
#endif
}