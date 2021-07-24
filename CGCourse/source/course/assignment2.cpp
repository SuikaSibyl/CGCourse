#include "AssignmentIndex.h"
#include <assert.h>
#include <string>
#include <RayTracer/Core/scene_parser.h>
#include <Image/image.h>
#include <RayTracer/Core/Camera.h>
#include <RayTracer/Primitives/Group.h>
#include <limits>
#include <RayTracer/Core/light.h>

#define DEPTHMAP(x) if(depth_file!=NULL){x}
#define NORMALMAP(x) if(normal_file!=NULL){x}

#pragma warning(disable:4996)
int Assignment::Assignment2Main(int argc, char* argv[])
{

#if (RTVersion==2)
    //raytracer -input scene1_01.txt -size 200 200 -output output1_01.tga -depth 9 10 depth1_01.tga

    char* input_file = NULL;
    int num_points = 10000;
    int num_iters = 10;
    int size_width = 0, size_height = 0;
    char* output_file = NULL;
    char* depth_file = NULL;
    char* normal_file = NULL;
    float depth_min = 0, depth_max = 0, depth_rerange = 1;

    bool shadeback = false;

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
        else {
            printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
            assert(0);
        }
    }

    // ========================================================
    // ========================================================
    // open the file
    std::string file_path = "./resource/assignment2/";
    std::string file_input = input_file;

    SceneParser scene((file_path + file_input).c_str());
    Image pImg = Image(size_width, size_height);
    Image pImgD = Image(size_width, size_height);
    Image pImgN = Image(size_width, size_height);
    
    pImg.SetAllPixels(scene.getBackgroundColor());
    Vec3f black(0, 0, 0);
    pImgD.SetAllPixels(black);
    NORMALMAP(pImgN.SetAllPixels(black);)
    Material black_mat(black);

    // prepare
    Camera* camera = scene.getCamera();
    Group* group = scene.getGroup();
    float tmin = camera->getTMin();

    // Rendering
    float step_width = 1. / size_width;
    float step_height = 1. / size_height;
    for (int i = 0; i < size_width; i++)
        for (int j = 0; j < size_height; j++)
        {
            Hit hit((float)numeric_limits<float>::max(), &black_mat, Vec3f(0, 0, 0));
            Ray r = camera->generateRay(Vec2f(step_width * i, step_height * j));
            bool intersected = group->intersect(r, hit, tmin);
            if (intersected)
            {
                // Shading
                Vec3f radiance(0, 0, 0);
                Vec3f albedo = hit.getMaterial()->getDiffuseColor();
                Vec3f::Mult(radiance, albedo, scene.getAmbientLight());
                Vec3f position = hit.getIntersectionPoint();
                Vec3f normal = hit.getNormal();
                if (!shadeback && normal.Dot3(r.getDirection()) > 0)
                {
                    pImg.SetPixel(i, j, black);
                    continue;
                }

                for (int k = 0; k < scene.getNumLights(); k++)
                {
                    Light* light = scene.getLight(k);
                    Vec3f dir, col;
                    light->getIllumination(position, dir, col);
                    Vec3f diffuse(0, 0, 0);
                    Vec3f::Mult(diffuse, albedo, col);
                    float cosine = normal.Dot3(dir);
                    diffuse *= shadeback ? abs(normal.Dot3(dir)) : max(normal.Dot3(dir), 0.0f);
                    radiance += diffuse;
                }

                pImg.SetPixel(i, j, radiance);

                NORMALMAP(
                    pImgN.SetPixel(i, j, Vec3f(abs(normal.x()), abs(normal.y()), abs(normal.z())));
                )

                DEPTHMAP(
                    float depth = hit.getT();
                    depth = (depth - depth_min) * depth_rerange;
                    if (depth > 1) depth = 1;
                    depth = 1 - depth;
                    pImgD.SetPixel(i, j, Vec3f(depth, depth, depth));
                )
            }
        }

    // Output
    assert(output_file != NULL);
    pImg.SaveTGA((string("resource/output/") + string(output_file)).c_str());
    DEPTHMAP(pImgD.SaveTGA((string("resource/output/") + string(depth_file)).c_str());)
    NORMALMAP(pImgN.SaveTGA((string("resource/output/") + string(normal_file)).c_str());)
    return 0;
#else
    std::cout << "Please switch to RayTracer History Version 2" << endl;
    return 1;
#endif
}