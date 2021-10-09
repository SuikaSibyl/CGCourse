#include "AssignmentIndex.h"

#include "../module/SplineEditor/ArgParser.h"
#include "../module/SplineEditor/SplineParser.h"
#include "../module/SplineEditor/glCanvasSE.h"
#include "../Settings.h"

#pragma warning(disable:4996)
int Assignment::Assignment8Main(int argc, char* argv[])
{
#if (RTVersion==8)
    // parse the command line arguments & input file
    ArgParser* args = new ArgParser(argc, argv);
    SplineParser* splines = new SplineParser(args->input_file);

    // launch curve editor!
    if (args->gui) {
        GLCanvasCurveEditor glcanvas;
        glcanvas.initialize(args, splines);
        // this never returns...
    }

    // output as required
    splines->SaveBezier(args);
    splines->SaveBSpline(args);
    splines->SaveTriangles(args);

    // cleanup
    delete args;
    delete splines;
#endif
    return 0;
}