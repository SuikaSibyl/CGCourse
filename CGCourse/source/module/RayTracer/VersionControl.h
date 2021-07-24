#pragma once
#include "../../Settings.h"

// If in assignment 1
#if (RTVersion==0 | RTVersion==1)
	#define Hit Hit_v1
	#define SceneParser SceneParser_v1
#endif // (RTVersion==0)

// If in assignment 2
#if (RTVersion==2)
	#define Hit Hit_v2
	#define SceneParser SceneParser_v2
#endif // (RTVersion==0)

// If in assignment 2
#if (RTVersion==3)
	#define Hit Hit_v2
	#define SceneParser SceneParser_v3
#endif // (RTVersion==0)

#define M_PI 3.1415926535898