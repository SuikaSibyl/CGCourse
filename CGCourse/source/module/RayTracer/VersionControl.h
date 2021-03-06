#pragma once
#include "../../Settings.h"

// If in assignment 1
#if (RTVersion==0 | RTVersion==1)
	#define Hit Hit_v1
	#define PhongMaterial PhongMaterial_v1
	#define SceneParser SceneParser_v1
#endif // (RTVersion==0)

// If in assignment 2
#if (RTVersion==2)
	#define Hit Hit_v2
	#define PhongMaterial PhongMaterial_v1
	#define SceneParser SceneParser_v2
#endif // (RTVersion==2)

// If in assignment 3
#if (RTVersion==3)
	#define Hit Hit_v2
	#define PhongMaterial PhongMaterial_v1
	#define SceneParser SceneParser_v3
#endif // (RTVersion==3)

// If in assignment 4
#if (RTVersion==4)
#define Hit Hit_v2
#define SceneParser SceneParser_v4
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==4)

// If in assignment 5
#if (RTVersion==5)
#define Hit Hit_v2
#define SceneParser SceneParser_v4
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==5)

// If in assignment 6
#if (RTVersion==6)
#define Hit Hit_v2
#define SceneParser SceneParser_v6
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==6)

// If in assignment 7
#if (RTVersion==7)
#define Hit Hit_v2
#define SceneParser SceneParser_v6
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==7)

// If in assignment 8
#if (RTVersion==8)
#define Hit Hit_v2
#define SceneParser SceneParser_v6
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==8)

// If in assignment 9
#if (RTVersion==9)
#define Hit Hit_v2
#define SceneParser SceneParser_v6
#define PhongMaterial PhongMaterial_v2
#endif // (RTVersion==9)

#define M_PI 3.1415926535898