#include "AssignmentIndex.h"
#include <assert.h>
#include <string>
#include <RayTracer/Core/scene_parser.h>
#include <Image/image.h>
#include <RayTracer/Core/Camera.h>
#include <RayTracer/Primitives/Group.h>
#include <RayTracer/Primitives/Sphere.h>
#include <limits>
#include <RayTracer/Core/light.h>
#include <OpenGL/Core/GLCanvas.h>

#if (RTVersion==4)

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

SceneParser* scene;

void Render()
{

    Image pImg = Image(size_width, size_height);
    Image pImgD = Image(size_width, size_height);
    Image pImgN = Image(size_width, size_height);

    pImg.SetAllPixels(scene->getBackgroundColor());
    Vec3f black(0, 0, 0);
    pImgD.SetAllPixels(black);
    NORMALMAP(pImgN.SetAllPixels(black););
    Material* black_mat = new PhongMaterial(black, black, 0);

    // prepare
    Camera* camera = scene->getCamera();
    Group* group = scene->getGroup();
    float tmin = camera->getTMin();

    // Rendering
    float step_width = 1. / size_width;
    float step_height = 1. / size_height;
    for (int i = 0; i < size_width; i++)
        for (int j = 0; j < size_height; j++)
        {
            Hit hit((float)numeric_limits<float>::max(), black_mat, Vec3f(0, 0, 0));
            Ray r = camera->generateRay(Vec2f(step_width * i, step_height * j));
            bool intersected = group->intersect(r, hit, tmin);
            if (intersected)
            {
                // Shading
                Vec3f radiance(0, 0, 0);
                Vec3f albedo = hit.getMaterial()->getDiffuseColor();
                Vec3f::Mult(radiance, albedo, scene->getAmbientLight());
                Vec3f position = hit.getIntersectionPoint();
                Vec3f normal = hit.getNormal();
                if (!shadeback && normal.Dot3(r.getDirection()) > 0)
                {
                    pImg.SetPixel(i, j, black);
                    continue;
                }

                for (int k = 0; k < scene->getNumLights(); k++)
                {
                    Light* light = scene->getLight(k);
                    Vec3f dir, col;
                    light->getIllumination(position, dir, col);
                    Vec3f outrad = hit.getMaterial()->Shade(r, hit, dir, col);
                    radiance += outrad;
                }

                pImg.SetPixel(i, j, radiance);

                NORMALMAP(
                    pImgN.SetPixel(i, j, Vec3f(abs(normal.x()), abs(normal.y()), abs(normal.z())));
                );

                DEPTHMAP(
                    float depth = hit.getT();
                depth = (depth - depth_min) * depth_rerange;
                if (depth > 1) depth = 1;
                depth = 1 - depth;
                pImgD.SetPixel(i, j, Vec3f(depth, depth, depth));
                );
            }
        }

    // Output
    assert(output_file != NULL);
    pImg.SaveTGA((string("resource/output/") + string(output_file)).c_str());
    DEPTHMAP(pImgD.SaveTGA((string("resource/output/") + string(depth_file)).c_str());)
        NORMALMAP(pImgN.SaveTGA((string("resource/output/") + string(normal_file)).c_str());)
}
#endif

#pragma warning(disable:4996)
int Assignment::Assignment4Main(int argc, char* argv[])
{

#if (RTVersion==4)

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
        else {
            printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
            assert(0);
        }
    }

    // ========================================================
    // ========================================================
    // open the file
    std::string file_path = "./resource/assignment3/";
    std::string file_input = input_file;

    scene = new SceneParser((file_path + file_input).c_str());
    if (useGUI)
    {
        GLCanvas canvas;
        canvas.initialize(scene, Render);
    }
    else
    {
        Render();
    }
    return 0;

#else
    std::cout << "Please switch to RayTracer History Version 4" << endl;
    return 1;
#endif
}