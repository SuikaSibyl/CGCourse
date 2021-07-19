#include <iostream>
#include <assert.h>
#include <matrix.h>
#include <string>
#include "AssignmentIndex.h"
#include "assignment0/IFS.h"
#pragma warning(disable:4996)

int Assignment::Assignment0Main(int argc, char* argv[])
{
// ========================================================
// ========================================================
// Some sample code you might like to use for parsing 
// command line arguments and the input IFS files

// sample command line:
// ifs -input fern.txt -points 10000 -iters 10 -size 100 -output fern.tga

    char* input_file = NULL;
    int num_points = 10000;
    int num_iters = 10;
    int size = 100;
    char* output_file = NULL;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-input")) {
            i++; assert(i < argc);
            input_file = argv[i];
        }
        else if (!strcmp(argv[i], "-points")) {
            i++; assert(i < argc);
            num_points = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-iters")) {
            i++; assert(i < argc);
            num_iters = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-size")) {
            i++; assert(i < argc);
            size = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-output")) {
            i++; assert(i < argc);
            output_file = argv[i];
        }
        else {
            printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
            assert(0);
        }
    }

    // ========================================================
    // ========================================================
    // Some sample code you might like to use for
    // parsing the IFS transformation files

    // open the file
    std::string file_path = "./resource/assignment0/";
    std::string file_input = input_file;
    FILE* input = fopen((file_path + file_input).c_str(), "r");
    assert(input != NULL);

    // read the number of transforms
    int num_transforms;
    fscanf(input, "%d", &num_transforms);

    // < DO SOMETHING WITH num_transforms >
    ProbabilityPair<Matrix>* ppair = new ProbabilityPair<Matrix>();

    // read in the transforms
    for (int i = 0; i < num_transforms; i++) {
        float probability;
        fscanf(input, "%f", &probability);
        Matrix m;
        m.Read3x3(input);
        ppair->PushPair(probability, m);
    }

    assert(output_file != NULL);
    IFS ifs(num_iters, num_points, size, string(output_file), ppair);

    // close the file
    fclose(input);

    // ========================================================
    // ========================================================

    return 0;
}