#include "AssignmentIndex.h"

#include "../Settings.h"

#include <stdio.h>
#include <assert.h>
using namespace std;

#include "../module/ParticleSystem/glCanvasPS.h"
#include "../module/ParticleSystem/Parser.h"

int Assignment::Assignment9Main(int argc, char* argv[])
{    
    // command line argument defaults
    const char* filename = NULL;
    float refresh = 0.1;  // in seconds
    float dt = 0.1;       // in seconds
    int integrator_color = 0;
    int draw_vectors = 0;
    float acceleration_scale = 1;
    int motion_blur = 0;

    std::string path(".\\resource\\assignment9\\");

    // parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-input")) {
            i++; assert(i < argc);
            path += std::string(argv[i]);
            filename = path.c_str();
        }
        else if (!strcmp(argv[i], "-refresh")) {
            i++; assert(i < argc);
            refresh = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-dt")) {
            i++; assert(i < argc);
            dt = atof(argv[i]);
        }
        else if (!strcmp(argv[i], "-integrator_color")) {
            integrator_color = 1;
        }
        else if (!strcmp(argv[i], "-motion_blur")) {
            motion_blur = 1;
        }
        else if (!strcmp(argv[i], "-draw_vectors")) {
            draw_vectors = 1;
            i++; assert(i < argc);
            acceleration_scale = atof(argv[i]);
        }
        else {
            printf("WARNING:  unknown command line argument %s\n", argv[i]);
            assert(0);
        }
    }

    // load the particle system
    assert(filename != NULL);
    Parser* parser = new Parser(filename);

    // launch viewer!   and it never returns...
    GLCanvasPS glcanvas;
    glcanvas.initialize(parser, refresh, dt, integrator_color,
        draw_vectors, acceleration_scale, motion_blur);

    return 0;
}
