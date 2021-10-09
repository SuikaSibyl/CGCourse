#include "AssignmentIndex.h"
#include <assert.h>
#include <string>
#include <RayTracer/Core/scene_parser.h>
#include <Image/image.h>
#include <RayTracer/Core/Camera.h>
#include <RayTracer/Core/RayTracer.h>
#include <RayTracer/Primitives/Group.h>
#include <RayTracer/Primitives/Sphere.h>
#include <limits>
#include <RayTracer/Core/light.h>
#include <OpenGL/Core/GLCanvas.h>
#include <RayTracer/Core/RayTracingStas.h>

#if (RTVersion==6)

#define DEPTHMAP(x) if(depth_file!=NULL){x}
#define NORMALMAP(x) if(normal_file!=NULL){x}

extern char* input_file = NULL;
extern int num_points = 10000;
extern int num_iters = 10;
extern int size_width = 0, size_height = 0;
extern char* output_file = NULL;
extern char* depth_file = NULL;
extern char* normal_file = NULL;
extern float depth_min = 0, depth_max = 0, depth_rerange = 1;

bool shadeback = false;
bool shadows = false;
bool bounces = false;
bool weight = false;
bool stats = false;
int nBounce = 0;
float fWeight = 0;

SceneParser_v6* scene;
RayTracer* rayTracer;

Vec3f black(0, 0, 0);
Material* black_mat = new PhongMaterial(black, black, 0, black, black, 1);

void TraceRay(float a, float b)
{
    Hit hit((float)numeric_limits<float>::max(), black_mat, Vec3f(0, 0, 0));
    Camera* camera = scene->getCamera();
    Ray r = camera->generateRay(Vec2f(a, b));
    Vec3f radiance = rayTracer->traceRay(r, 0, 5, 0, 5, hit);
}

void Render()
{
    Image pImg = Image(size_width, size_height);
    Image pImgD = Image(size_width, size_height);
    Image pImgN = Image(size_width, size_height);

    int mx = -1, my = -1, mz = -1;
    if (scene->grid != nullptr)
        scene->grid->GetGridSegments(mx, my, mz);

    RayTracingStats::Initialize(size_width, size_height, 
        scene->getGroup()->getBoundingBox(),
        mx, my, mz);

    pImg.SetAllPixels(scene->getBackgroundColor());
    pImgD.SetAllPixels(black);
    NORMALMAP(pImgN.SetAllPixels(black););

    // prepare
    Camera* camera = scene->getCamera();
    Group* group = scene->getGroup();
    float tmin = camera->getTMin();

    // Rendering
    float step_width = 1. / size_width;
    float step_height = 1. / size_height;
    float hdw = 1. * size_height / size_width;
    float start_height = 0.5 - hdw * 0.5;
    for (int i = 0; i < size_width; i++)
        for (int j = 0; j < size_height; j++)
        {
            Hit hit((float)numeric_limits<float>::max(), black_mat, Vec3f(0, 0, 0));
            Ray r = camera->generateRay(Vec2f(step_width * i, start_height + hdw * step_height * j));
            Vec3f radiance = rayTracer->traceRay(r, 0, 5, 0, 5, hit);

            if (radiance == Vec3f(-1, -1, -1))
            {

            }
            else
            {
                pImg.SetPixel(i, j, radiance);
            }
        }

    if (stats)
    {
        RayTracingStats::PrintStatistics();
    }
    // Output
    assert(output_file != NULL);
    pImg.SaveTGA((string("resource/output/") + string(output_file)).c_str());
    DEPTHMAP(pImgD.SaveTGA((string("resource/output/") + string(depth_file)).c_str());)
        NORMALMAP(pImgN.SaveTGA((string("resource/output/") + string(normal_file)).c_str());)
}
#endif

#pragma warning(disable:4996)
int Assignment::Assignment6Main(int argc, char* argv[])
{

#if (RTVersion==6)

    bool useGUI = false;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-input")) {
            i++; assert(i < argc);
            input_file = argv[i];
        }
        else if (!strcmp(argv[i], "-size")) {
            i++; assert(i < argc);
            size_width = atoi(argv[i]);
            i++; assert(i < argc);
            size_height = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-output")) {
            i++; assert(i < argc);
            output_file = argv[i];
        }
        else if (!strcmp(argv[i], "-depth")) {
            i++; assert(i < argc);
            depth_min = atof(argv[i]);
            i++; assert(i < argc);
            depth_max = atof(argv[i]);
            i++; assert(i < argc);
            depth_file = argv[i];
            depth_rerange = 1. / (depth_max - depth_min);
        }
        else if (!strcmp(argv[i], "-normals")) {
            i++; assert(i < argc);
            normal_file = argv[i];
        }
        else if (!strcmp(argv[i], "-shade_back")) {
            shadeback = true;
        }
        else if (!strcmp(argv[i], "-gui")) {
            useGUI = true;
        }
        else if (!strcmp(argv[i], "-tessellation")) {
            i++; assert(i < argc);
            tessx = atoi(argv[i]);
            i++; assert(i < argc);
            tessy = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-gouraud")) {
            gouraud = true;
        }
        else if (!strcmp(argv[i], "-shadows")) {
            shadows = true;
        }
        else if (!strcmp(argv[i], "-bounces")) {
            bounces = true;
            i++; assert(i < argc);
            nBounce = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-weight")) {
            weight = true;
            i++; assert(i < argc);
            fWeight = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-grid")) {
            grid = true;
            i++; assert(i < argc);
            gridx = atoi(argv[i]);
            i++; assert(i < argc);
            gridy = atoi(argv[i]);
            i++; assert(i < argc);
            gridz = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-visualize_grid")) {
            visualize_grid = true;
        }
        else if (!strcmp(argv[i], "-stats")) {
            stats = true;
        }
        else {
            printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
            assert(0);
        }
    }

    // ========================================================
    // ========================================================
    // open the file
    std::string file_path = "./resource/assignment6/";
    std::string file_input = input_file;

    scene = new SceneParser_v6((file_path + file_input).c_str());
    rayTracer = new RayTracer(scene, nBounce, fWeight, shadows);

    if (useGUI)
    {
        GLCanvas canvas;
        canvas.initialize(scene, Render, TraceRay, rayTracer->GetGrid(), visualize_grid);
    }
    else
    {
        Render();
    }
    return 0;

#else
    std::cout << "Please switch to RayTracer History Version 5" << endl;
    return 1;
#endif
}