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
#include <RayTracer/AntiAliasing/Film.h>
#include <RayTracer/AntiAliasing/Sampler.h>
#include <RayTracer/AntiAliasing/Filter.h>

#if (RTVersion==7)

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

enum class SampleType
{
    None,
    RandomSample,
    UniformSample,
    JitteredSample,
};
enum class FilterType
{
    None,
    BoxFilter,
    TentFilter,
    GaussianFilter,
};
bool useSuperSampling = false;
bool renderFilter = false;
int spp = 1;
float filterRadius = 0;
int render_filterZoomFactor = 0;
char* render_filterfile = NULL;
int render_sampleZoomFactor = 0;
char* render_samplesFile = NULL;
int sampleNumber = 0;
SampleType sampleType = SampleType::None;
FilterType filterType = FilterType::None;

Film* pFilm = nullptr;
Sampler* sampler = nullptr;
Filter* filter = nullptr;

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

    int mx = -1, my = -1, mz = -1;
    if (scene->grid != nullptr)
        scene->grid->GetGridSegments(mx, my, mz);

    RayTracingStats::Initialize(size_width, size_height,
        scene->getGroup()->getBoundingBox(),
        mx, my, mz);

    pImg.SetAllPixels(scene->getBackgroundColor());

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
            switch (sampleType)
            {
            case SampleType::None:
            {
                Vec2f offset(0.5, 0.5);

                Hit hit((float)numeric_limits<float>::max(), black_mat, Vec3f(0, 0, 0));
                Ray r = camera->generateRay(Vec2f(
                    step_width * (i + offset.x()),
                    start_height + hdw * step_height * (j + offset.y())));
                Vec3f radiance = rayTracer->traceRay(r, 0, 5, 0, 5, hit);
                if (radiance == Vec3f(-1, -1, -1))
                    radiance = scene->getBackgroundColor();
                pFilm->setSample(i, j, 0, offset, radiance);
                pImg.SetPixel(i, j, radiance);
            }
            break;
            case SampleType::RandomSample:
            case SampleType::UniformSample:
            case SampleType::JitteredSample:
            {
                for (int k = 0; k < spp; k++)
                {
                    Vec2f offset = sampler->getSamplePosition(k);

                    Hit hit((float)numeric_limits<float>::max(), black_mat, Vec3f(0, 0, 0));
                    Ray r = camera->generateRay(Vec2f(
                        step_width * (i + offset.x()),
                        start_height + hdw * step_height * (j + offset.y())));
                    Vec3f radiance = rayTracer->traceRay(r, 0, 5, 0, 5, hit);
                    if (radiance == Vec3f(-1, -1, -1))
                        radiance = scene->getBackgroundColor();
                    pFilm->setSample(i, j, k, offset, radiance);
                }
            }
            break;
            default:
                break;
            }
        }

    if (filterType != FilterType::None)
        for (int i = 0; i < size_width; i++)
            for (int j = 0; j < size_height; j++)
            {
                Vec3f color = filter->getColor(i, j, pFilm);
                pImg.SetPixel(i, j, color);
            }

    if (stats)
    {
        RayTracingStats::PrintStatistics();
    }
    // Output
    if (output_file != NULL)
    {
        pImg.SaveTGA((string("resource/output/") + string(output_file)).c_str());
    }

    if (render_samplesFile != NULL)
    {
        pFilm->renderSamples((string("resource/output/") + string(render_samplesFile)).c_str(), render_sampleZoomFactor);

    }

    if (renderFilter != NULL)
    {
        pFilm->renderFilter((string("resource/output/") + string(render_filterfile)).c_str(), render_filterZoomFactor, filter);
    }
}
#endif

#pragma warning(disable:4996)
int Assignment::Assignment7Main(int argc, char* argv[])
{
#if (RTVersion==7)

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
        else if (!strcmp(argv[i], "-box_filter")) {
            filterType = FilterType::BoxFilter;
            i++; assert(i < argc);
            filterRadius = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-tent_filter")) {
            filterType = FilterType::TentFilter;
            i++; assert(i < argc);
            filterRadius = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-gaussian_filter")) {
            filterType = FilterType::GaussianFilter;
            i++; assert(i < argc);
            filterRadius = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-random_samples")) {
            sampleType = SampleType::RandomSample;
            i++; assert(i < argc);
            spp = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-uniform_samples")) {
            sampleType = SampleType::UniformSample;
            i++; assert(i < argc);
            spp = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-jittered_samples")) {
            sampleType = SampleType::JitteredSample;
            i++; assert(i < argc);
            spp = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-render_samples")) {
            i++; assert(i < argc);
            render_samplesFile = argv[i];
            i++; assert(i < argc);
            render_sampleZoomFactor = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-render_filter")) {
            renderFilter = true;
            i++; assert(i < argc);
            render_filterfile = argv[i];
            i++; assert(i < argc);
            render_filterZoomFactor = atoi(argv[i]);
        }
        else {
            printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
            assert(0);
        }
    }

    // ========================================================
    // ========================================================
    // open the file
    std::string file_path = "./resource/assignment7/";
    std::string file_input = input_file;

    scene = new SceneParser_v6((file_path + file_input).c_str());
    rayTracer = new RayTracer(scene, nBounce, fWeight, shadows);
    if (spp != 0) pFilm = new Film(size_width, size_height, spp);

    switch (sampleType)
    {
    case SampleType::None:
        break;
    case SampleType::RandomSample:
        sampler = new RandomSampler(spp);
        break;
    case SampleType::UniformSample:
        sampler = new UniformSampler(spp);
        break;
    case SampleType::JitteredSample:
        sampler = new JitteredSampler(spp);
        break;
    default:
        break;
    }

    switch (filterType)
    {
    case FilterType::None:
        break;
    case FilterType::BoxFilter:
        filter = new BoxFilter(filterRadius);
        break;
    case FilterType::TentFilter:
        filter = new TentFilter(filterRadius);
        break;
    case FilterType::GaussianFilter:
        filter = new GaussianFilter(filterRadius);
        break;
    default:
        break;
    }

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