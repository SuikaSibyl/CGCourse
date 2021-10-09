#include <stdio.h>
#include <string.h>

#include "scene_parser.h"
#include "matrix.h"
#include "camera.h" 
#include "material.h"
#include "object3d.h"
#include "light.h"
#include "../Primitives/Group.h"
#include "../Primitives/Sphere.h"
#include "../Primitives/Plane.h"
#include "../Primitives/Triangle.h"
#include "../Primitives/Transform.h"
#include "../VersionControl.h"
#include "Noise.h"
#include "Checkerboard.h"
#include "../Materials/Marble.h"
#include "../Materials/Wood.h"

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

#pragma region V6

// ====================================================================
// ====================================================================
// CONSTRUCTOR & DESTRUCTOR

SceneParser_v6::SceneParser_v6(const char* filename) {

    // initialize some reasonable default values
    group = NULL;
    camera = NULL;
    background_color = Vec3f(0.5, 0.5, 0.5);
    ambient_light = Vec3f(0, 0, 0);
    num_lights = 0;
    lights = NULL;
    num_materials = 0;
    materials = NULL;
    current_material = NULL;

    // parse the file
    assert(filename != NULL);
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".txt"));
    file = fopen(filename, "r");
    assert(file != NULL);
    parseFile();
    fclose(file);
    file = NULL;

    // if no lights are specified, set ambient light to white
    // (do solid color ray casting)
    if (num_lights == 0) {
        printf("WARNING:  No lights specified\n");
        ambient_light = Vec3f(1, 1, 1);
    }
}

SceneParser_v6::~SceneParser_v6() {
    if (group != NULL)
        delete group;
    if (camera != NULL)
        delete camera;
    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser_v6::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "OrthographicCamera")) {
            parseOrthographicCamera();
        }
        else if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Lights")) {
            parseLights();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v6::parseOrthographicCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "size"));
    float size = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    camera = new OrthographicCamera(center, direction, up, size);
}


void SceneParser_v6::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token); assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

void SceneParser_v6::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVec3f();
        }
        else if (!strcmp(token, "ambientLight")) {
            ambient_light = readVec3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v6::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light * [num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (!strcmp(token, "DirectionalLight")) {
            lights[count] = parseDirectionalLight();
        }
        else if (!strcmp(token, "PointLight")) {
            lights[count] = parsePointLight();
        }
        else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Light* SceneParser_v6::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}


Light* SceneParser_v6::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "position"));
    Vec3f position = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    float att[3] = { 1, 0, 0 };
    getToken(token);
    if (!strcmp(token, "attenuation")) {
        att[0] = readFloat();
        att[1] = readFloat();
        att[2] = readFloat();
        getToken(token);
    }
    assert(!strcmp(token, "}"));
    return new PointLight(position, color, att[0], att[1], att[2]);
}

// ====================================================================
// ====================================================================

void SceneParser_v6::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material * [num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial")) {
            materials[count] = parsePhongMaterial();
        }
        else if (!strcmp(token, "Checkerboard")) {
            materials[count] = parseCheckerboard(count);
        }
        else if (!strcmp(token, "Noise")) {
            materials[count] = parseNoise(count);
        }
        else if (!strcmp(token, "Marble")) {
            materials[count] = parseMarble(count);
        }
        else if (!strcmp(token, "Wood")) {
            materials[count] = parseWood(count);
        }
        else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Material* SceneParser_v6::parsePhongMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3f diffuseColor(1, 1, 1);
    Vec3f specularColor(0, 0, 0);
    float exponent = 1;
    Vec3f reflectiveColor(0, 0, 0);
    Vec3f transparentColor(0, 0, 0);
    float indexOfRefraction = 1;
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "diffuseColor")) {
            diffuseColor = readVec3f();
        }
        else if (!strcmp(token, "specularColor")) {
            specularColor = readVec3f();
        }
        else if (!strcmp(token, "exponent")) {
            exponent = readFloat();
        }
        else if (!strcmp(token, "reflectiveColor")) {
            reflectiveColor = readVec3f();
        }
        else if (!strcmp(token, "transparentColor")) {
            transparentColor = readVec3f();
        }
        else if (!strcmp(token, "indexOfRefraction")) {
            indexOfRefraction = readFloat();
        }
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new PhongMaterial(diffuseColor, specularColor, exponent,
        reflectiveColor, transparentColor,
        indexOfRefraction);
    return answer;
}

Material* SceneParser_v6::parseCheckerboard(int count) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    Matrix* matrix = NULL;
    getToken(token);
    if (!strcmp(token, "Transform")) {
        matrix = new Matrix();
        matrix->SetToIdentity();
        getToken(token); assert(!strcmp(token, "{"));
        parseMatrixHelper(*matrix, token);
        assert(!strcmp(token, "}"));
        getToken(token);
    }
    assert(!strcmp(token, "materialIndex"));
    int m1 = readInt();
    assert(m1 >= 0 && m1 < count);
    getToken(token); assert(!strcmp(token, "materialIndex"));
    int m2 = readInt();
    assert(m2 >= 0 && m2 < count);
    getToken(token); assert(!strcmp(token, "}"));
    return new Checkerboard(matrix, materials[m1], materials[m2]);
}


Material* SceneParser_v6::parseNoise(int count) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    Matrix* matrix = NULL;
    getToken(token);
    if (!strcmp(token, "Transform")) {
        matrix = new Matrix();
        matrix->SetToIdentity();
        getToken(token); assert(!strcmp(token, "{"));
        parseMatrixHelper(*matrix, token);
        assert(!strcmp(token, "}"));
        getToken(token);
    }
    assert(!strcmp(token, "materialIndex"));
    int m1 = readInt();
    assert(m1 >= 0 && m1 < count);
    getToken(token); assert(!strcmp(token, "materialIndex"));
    int m2 = readInt();
    assert(m2 >= 0 && m2 < count);
    getToken(token); assert(!strcmp(token, "octaves"));
    int octaves = readInt();
    getToken(token); assert(!strcmp(token, "}"));
    return new Noise(matrix, materials[m1], materials[m2], octaves);
}

Material* SceneParser_v6::parseMarble(int count) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    Matrix* matrix = NULL;
    getToken(token);
    if (!strcmp(token, "Transform")) {
        matrix = new Matrix();
        matrix->SetToIdentity();
        getToken(token); assert(!strcmp(token, "{"));
        parseMatrixHelper(*matrix, token);
        assert(!strcmp(token, "}"));
        getToken(token);
    }
    assert(!strcmp(token, "materialIndex"));
    int m1 = readInt();
    assert(m1 >= 0 && m1 < count);
    getToken(token); assert(!strcmp(token, "materialIndex"));
    int m2 = readInt();
    assert(m2 >= 0 && m2 < count);
    getToken(token); assert(!strcmp(token, "octaves"));
    int octaves = readInt();
    getToken(token); assert(!strcmp(token, "frequency"));
    float frequency = readFloat();
    getToken(token); assert(!strcmp(token, "amplitude"));
    float amplitude = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    return new Marble(matrix, materials[m1], materials[m2], octaves, frequency, amplitude);
}


Material* SceneParser_v6::parseWood(int count) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    Matrix* matrix = NULL;
    getToken(token);
    if (!strcmp(token, "Transform")) {
        matrix = new Matrix();
        matrix->SetToIdentity();
        getToken(token); assert(!strcmp(token, "{"));
        parseMatrixHelper(*matrix, token);
        assert(!strcmp(token, "}"));
        getToken(token);
    }
    assert(!strcmp(token, "materialIndex"));
    int m1 = readInt();
    assert(m1 >= 0 && m1 < count);
    getToken(token); assert(!strcmp(token, "materialIndex"));
    int m2 = readInt();
    assert(m2 >= 0 && m2 < count);
    getToken(token); assert(!strcmp(token, "octaves"));
    int octaves = readInt();
    getToken(token); assert(!strcmp(token, "frequency"));
    float frequency = readFloat();
    getToken(token); assert(!strcmp(token, "amplitude"));
    float amplitude = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    return new Wood(matrix, materials[m1], materials[m2], octaves, frequency, amplitude);
}


// ====================================================================
// ====================================================================

Object3D* SceneParser_v6::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else if (!strcmp(token, "Plane")) {
        answer = (Object3D*)parsePlane();
    }
    else if (!strcmp(token, "Triangle")) {
        answer = (Object3D*)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D*)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform")) {
        answer = (Object3D*)parseTransform();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser_v6::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    Group* answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token); assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser_v6::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Sphere(center, radius, current_material);
}


Plane* SceneParser_v6::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "normal"));
    Vec3f normal = readVec3f();
    getToken(token); assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Plane(normal, offset, current_material);
}


Triangle* SceneParser_v6::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vec3f v0 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vec3f v1 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vec3f v2 = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Triangle(v0, v1, v2, current_material);
}

Group* SceneParser_v6::parseTriangleMesh() {

    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token); assert(!strcmp(token, "}"));
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    // read it once, get counts
    char filepath[MAX_PARSER_TOKEN_LENGTH] = "./resource/mesh/";
    strcat(filepath, filename);

    FILE* file = fopen(filepath, "r");
    assert(file != NULL);
    int vcount = 0; int fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            vcount++;
        }
        else if (c == 'f') {
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            fcount++;
        } // otherwise, must be whitespace
    }
    fclose(file);
    // make arrays
    Vec3f* verts = new Vec3f[vcount];
    Group* answer = new Group(fcount);
    // read it again, save it
    file = fopen(filepath, "r");
    assert(file != NULL);
    int new_vcount = 0; int new_fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(new_fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            verts[new_vcount] = Vec3f(v0, v1, v2);
            new_vcount++;
        }
        else if (c == 'f') {
            assert(vcount == new_vcount);
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            // indexed starting at 1...
            assert(f0 > 0 && f0 <= vcount);
            assert(f1 > 0 && f1 <= vcount);
            assert(f2 > 0 && f2 <= vcount);
            assert(current_material != NULL);
            Triangle* t = new Triangle(verts[f0 - 1], verts[f1 - 1], verts[f2 - 1], current_material);
            answer->addObject(new_fcount, t);
            new_fcount++;
        } // otherwise, must be whitespace
    }
    delete[] verts;
    assert(fcount == new_fcount);
    assert(vcount == new_vcount);
    fclose(file);
    return answer;
}


Transform* SceneParser_v6::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix matrix; matrix.SetToIdentity();
    // opening brace
    getToken(token); assert(!strcmp(token, "{"));
    // the matrix
    parseMatrixHelper(matrix, token);
    // the Object3D
    Object3D* object = parseObject(token);
    assert(object != NULL);
    // closing brace
    getToken(token); assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}

void SceneParser_v6::parseMatrixHelper(Matrix& matrix, char token[MAX_PARSER_TOKEN_LENGTH]) {
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    while (1) {
        getToken(token);
        if (!strcmp(token, "Scale")) {
            matrix *= Matrix::MakeScale(readVec3f());
        }
        else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix *= Matrix::MakeScale(Vec3f(s, s, s));
        }
        else if (!strcmp(token, "Translate")) {
            matrix *= Matrix::MakeTranslation(readVec3f());
        }
        else if (!strcmp(token, "XRotate")) {
            matrix *= Matrix::MakeXRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate")) {
            matrix *= Matrix::MakeYRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate")) {
            matrix *= Matrix::MakeZRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate")) {
            getToken(token); assert(!strcmp(token, "{"));
            Vec3f axis = readVec3f();
            float degrees = readFloat();
            matrix *= Matrix::MakeAxisRotation(axis, DegreesToRadians(degrees));
            getToken(token); assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix")) {
            Matrix matrix2; matrix2.SetToIdentity();
            getToken(token); assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2.Set(i, j, v);
                }
            }
            getToken(token); assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        }
        else {
            // otherwise this must be the thing to transform
            break;
        }
    }
}


// ====================================================================
// ====================================================================

int SceneParser_v6::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != NULL);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3f SceneParser_v6::readVec3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vec3f\n");
        assert(0);
    }
    return Vec3f(x, y, z);
}


Vec2f SceneParser_v6::readVec2f() {
    float u, v;
    int count = fscanf(file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vec2f(u, v);
}


float SceneParser_v6::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser_v6::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================
#pragma endregion


#pragma region V4

// ====================================================================
// ====================================================================
// CONSTRUCTOR & DESTRUCTOR

SceneParser_v4::SceneParser_v4(const char* filename) {

    // initialize some reasonable default values
    group = NULL;
    camera = NULL;
    background_color = Vec3f(0.5, 0.5, 0.5);
    ambient_light = Vec3f(0, 0, 0);
    num_lights = 0;
    lights = NULL;
    num_materials = 0;
    materials = NULL;
    current_material = NULL;

    // parse the file
    assert(filename != NULL);
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".txt"));
    file = fopen(filename, "r");
    assert(file != NULL);
    parseFile();
    fclose(file);
    file = NULL;

    // if no lights are specified, set ambient light to white
    // (do solid color ray casting)
    if (num_lights == 0) {
        printf("WARNING:  No lights specified\n");
        ambient_light = Vec3f(1, 1, 1);
    }
}

SceneParser_v4::~SceneParser_v4() {
    if (group != NULL)
        delete group;
    if (camera != NULL)
        delete camera;
    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser_v4::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "OrthographicCamera")) {
            parseOrthographicCamera();
        }
        else if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Lights")) {
            parseLights();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v4::parseOrthographicCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "size"));
    float size = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    camera = new OrthographicCamera(center, direction, up, size);
}


void SceneParser_v4::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token); assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

void SceneParser_v4::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVec3f();
        }
        else if (!strcmp(token, "ambientLight")) {
            ambient_light = readVec3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v4::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light*[num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (!strcmp(token, "DirectionalLight")) {
            lights[count] = parseDirectionalLight();
        }
        else if (!strcmp(token, "PointLight")) {
            lights[count] = parsePointLight();
        }
        else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Light* SceneParser_v4::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}


Light* SceneParser_v4::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "position"));
    Vec3f position = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    float att[3] = { 1, 0, 0 };
    getToken(token);
    if (!strcmp(token, "attenuation")) {
        att[0] = readFloat();
        att[1] = readFloat();
        att[2] = readFloat();
        getToken(token);
    }
    assert(!strcmp(token, "}"));
    return new PointLight(position, color, att[0], att[1], att[2]);
}

// ====================================================================
// ====================================================================

void SceneParser_v4::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material*[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial_v1")) {
            materials[count] = parsePhongMaterial();
        }
        else if (!strcmp(token, "PhongMaterial"))
        {
            materials[count] = parsePhongMaterial();
        }
        else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Material* SceneParser_v4::parsePhongMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3f diffuseColor(1, 1, 1);
    Vec3f specularColor(0, 0, 0);
    float exponent = 1;
    Vec3f reflectiveColor(0, 0, 0);
    Vec3f transparentColor(0, 0, 0);
    float indexOfRefraction = 1;
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "diffuseColor")) {
            diffuseColor = readVec3f();
        }
        else if (!strcmp(token, "specularColor")) {
            specularColor = readVec3f();
        }
        else if (!strcmp(token, "reflectiveColor ")) {
            specularColor = readVec3f();
        }
        else if (!strcmp(token, "exponent")) {
            exponent = readFloat();
        }
        else if (!strcmp(token, "reflectiveColor")) {
            reflectiveColor = readVec3f();
        }
        else if (!strcmp(token, "transparentColor")) {
            transparentColor = readVec3f();
        }
        else if (!strcmp(token, "indexOfRefraction")) {
            indexOfRefraction = readFloat();
        }
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new PhongMaterial(diffuseColor, specularColor, exponent,
        reflectiveColor, transparentColor,
        indexOfRefraction);
    return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser_v4::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else if (!strcmp(token, "Plane")) {
        answer = (Object3D*)parsePlane();
    }
    else if (!strcmp(token, "Triangle")) {
        answer = (Object3D*)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D*)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform")) {
        answer = (Object3D*)parseTransform();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser_v4::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    Group* answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token); assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser_v4::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Sphere(center, radius, current_material);
}


Plane* SceneParser_v4::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "normal"));
    Vec3f normal = readVec3f();
    getToken(token); assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Plane(normal, offset, current_material);
}


Triangle* SceneParser_v4::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vec3f v0 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vec3f v1 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vec3f v2 = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Triangle(v0, v1, v2, current_material);
}

Group* SceneParser_v4::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token); assert(!strcmp(token, "}"));
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    // read it once, get counts
    char filepath[MAX_PARSER_TOKEN_LENGTH] = "./resource/mesh/";
    strcat(filepath, filename);

    FILE* file = fopen(filepath, "r");
    assert(file != NULL);
    int vcount = 0; int fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            vcount++;
        }
        else if (c == 'f') {
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            fcount++;
        } // otherwise, must be whitespace
    }
    fclose(file);
    // make arrays
    Vec3f* verts = new Vec3f[vcount];
    Group* answer = new Group(fcount);
    // read it again, save it
    file = fopen(filepath, "r");
    assert(file != NULL);
    int new_vcount = 0; int new_fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(new_fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            verts[new_vcount] = Vec3f(v0, v1, v2);
            new_vcount++;
        }
        else if (c == 'f') {
            assert(vcount == new_vcount);
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            // indexed starting at 1...
            assert(f0 > 0 && f0 <= vcount);
            assert(f1 > 0 && f1 <= vcount);
            assert(f2 > 0 && f2 <= vcount);
            assert(current_material != NULL);
            Triangle* t = new Triangle(verts[f0 - 1], verts[f1 - 1], verts[f2 - 1], current_material);
            answer->addObject(new_fcount, t);
            new_fcount++;
        } // otherwise, must be whitespace
    }
    delete[] verts;
    assert(fcount == new_fcount);
    assert(vcount == new_vcount);
    fclose(file);
    return answer;
}


Transform* SceneParser_v4::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix matrix; matrix.SetToIdentity();
    Object3D* object = NULL;
    getToken(token); assert(!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);
    while (1) {
        if (!strcmp(token, "Scale")) {
            matrix *= Matrix::MakeScale(readVec3f());
        }
        else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix *= Matrix::MakeScale(Vec3f(s, s, s));
        }
        else if (!strcmp(token, "Translate")) {
            matrix *= Matrix::MakeTranslation(readVec3f());
        }
        else if (!strcmp(token, "XRotate")) {
            matrix *= Matrix::MakeXRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate")) {
            matrix *= Matrix::MakeYRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate")) {
            matrix *= Matrix::MakeZRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate")) {
            getToken(token); assert(!strcmp(token, "{"));
            Vec3f axis = readVec3f();
            float degrees = readFloat();
            matrix *= Matrix::MakeAxisRotation(axis, DegreesToRadians(degrees));
            getToken(token); assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix")) {
            Matrix matrix2; matrix2.SetToIdentity();
            getToken(token); assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2.Set(i, j, v);
                }
            }
            getToken(token); assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        }
        else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }
    assert(object != NULL);
    getToken(token); assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}

// ====================================================================
// ====================================================================

int SceneParser_v4::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != NULL);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3f SceneParser_v4::readVec3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vec3f\n");
        assert(0);
    }
    return Vec3f(x, y, z);
}


Vec2f SceneParser_v4::readVec2f() {
    float u, v;
    int count = fscanf(file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vec2f(u, v);
}


float SceneParser_v4::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser_v4::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

#pragma endregion

#if (RTVersion < 4)
#pragma region DEPRECATED

// ====================================================================
// ====================================================================
// CONSTRUCTOR & DESTRUCTOR

SceneParser_v3::SceneParser_v3(const char* filename) {

    // initialize some reasonable default values
    group = NULL;
    camera = NULL;
    background_color = Vec3f(0.5, 0.5, 0.5);
    ambient_light = Vec3f(0, 0, 0);
    num_lights = 0;
    lights = NULL;
    num_materials = 0;
    materials = NULL;
    current_material = NULL;

    // parse the file
    assert(filename != NULL);
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".txt"));
    file = fopen(filename, "r");
    assert(file != NULL);
    parseFile();
    fclose(file);
    file = NULL;

    // if no lights are specified, set ambient light to white
    // (do solid color ray casting)
    if (num_lights == 0) {
        printf("WARNING:  No lights specified\n");
        ambient_light = Vec3f(1, 1, 1);
    }
}

SceneParser_v3::~SceneParser_v3() {
    if (group != NULL)
        delete group;
    if (camera != NULL)
        delete camera;
    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser_v3::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "OrthographicCamera")) {
            parseOrthographicCamera();
        }
        else if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Lights")) {
            parseLights();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v3::parseOrthographicCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "size"));
    float size = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    camera = new OrthographicCamera(center, direction, up, size);
}


void SceneParser_v3::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token); assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

void SceneParser_v3::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVec3f();
        }
        else if (!strcmp(token, "ambientLight")) {
            ambient_light = readVec3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v3::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light * [num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (!strcmp(token, "DirectionalLight")) {
            lights[count] = parseDirectionalLight();
        }
        else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Light* SceneParser_v3::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

// ====================================================================
// ====================================================================

void SceneParser_v3::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material * [num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial_v1")) {
            materials[count] = parsePhongMaterial();
        }
        else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Material* SceneParser_v3::parsePhongMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3f diffuseColor(1, 1, 1);
    Vec3f specularColor(0, 0, 0);
    float exponent = 1;
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "diffuseColor")) {
            diffuseColor = readVec3f();
        }
        else if (!strcmp(token, "specularColor")) {
            specularColor = readVec3f();
        }
        else if (!strcmp(token, "exponent")) {
            exponent = readFloat();
        }
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new PhongMaterial(diffuseColor, specularColor, exponent);
    return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser_v3::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else if (!strcmp(token, "Plane")) {
        answer = (Object3D*)parsePlane();
    }
    else if (!strcmp(token, "Triangle")) {
        answer = (Object3D*)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D*)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform")) {
        answer = (Object3D*)parseTransform();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser_v3::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    Group* answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token); assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser_v3::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Sphere(center, radius, current_material);
}


Plane* SceneParser_v3::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "normal"));
    Vec3f normal = readVec3f();
    getToken(token); assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Plane(normal, offset, current_material);
}


Triangle* SceneParser_v3::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vec3f v0 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vec3f v1 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vec3f v2 = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Triangle(v0, v1, v2, current_material);
}

Group* SceneParser_v3::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token); assert(!strcmp(token, "}"));
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    // read it once, get counts
    std::string path = "./resource/mesh/";
    std::string fname = filename;
    fname = path + fname;
    FILE* file = fopen((fname).c_str(), "r");
    assert(file != NULL);
    int vcount = 0; int fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            vcount++;
        }
        else if (c == 'f') {
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            fcount++;
        } // otherwise, must be whitespace
    }
    fclose(file);
    // make arrays
    Vec3f* verts = new Vec3f[vcount];
    Group* answer = new Group(fcount);
    // read it again, save it
    file = fopen((fname).c_str(), "r");
    assert(file != NULL);
    int new_vcount = 0; int new_fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(new_fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            verts[new_vcount] = Vec3f(v0, v1, v2);
            new_vcount++;
        }
        else if (c == 'f') {
            assert(vcount == new_vcount);
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            // indexed starting at 1...
            assert(f0 > 0 && f0 <= vcount);
            assert(f1 > 0 && f1 <= vcount);
            assert(f2 > 0 && f2 <= vcount);
            assert(current_material != NULL);
            Triangle* t = new Triangle(verts[f0 - 1], verts[f1 - 1], verts[f2 - 1], current_material);
            answer->addObject(new_fcount, t);
            new_fcount++;
        } // otherwise, must be whitespace
    }
    delete[] verts;
    assert(fcount == new_fcount);
    assert(vcount == new_vcount);
    fclose(file);
    return answer;
}


Transform* SceneParser_v3::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix matrix; matrix.SetToIdentity();
    Object3D* object = NULL;
    getToken(token); assert(!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);
    while (1) {
        if (!strcmp(token, "Scale")) {
            matrix *= Matrix::MakeScale(readVec3f());
        }
        else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix *= Matrix::MakeScale(Vec3f(s, s, s));
        }
        else if (!strcmp(token, "Translate")) {
            matrix *= Matrix::MakeTranslation(readVec3f());
        }
        else if (!strcmp(token, "XRotate")) {
            matrix *= Matrix::MakeXRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate")) {
            matrix *= Matrix::MakeYRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate")) {
            matrix *= Matrix::MakeZRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate")) {
            getToken(token); assert(!strcmp(token, "{"));
            Vec3f axis = readVec3f();
            float degrees = readFloat();
            matrix *= Matrix::MakeAxisRotation(axis, DegreesToRadians(degrees));
            getToken(token); assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix")) {
            Matrix matrix2; matrix2.SetToIdentity();
            getToken(token); assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2.Set(i, j, v);
                }
            }
            getToken(token); assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        }
        else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }
    assert(object != NULL);
    getToken(token); assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}

// ====================================================================
// ====================================================================

int SceneParser_v3::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != NULL);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3f SceneParser_v3::readVec3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vec3f\n");
        assert(0);
    }
    return Vec3f(x, y, z);
}


Vec2f SceneParser_v3::readVec2f() {
    float u, v;
    int count = fscanf(file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vec2f(u, v);
}


float SceneParser_v3::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser_v3::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}

#pragma region V2
#if (RTVersion==2)

// ====================================================================
// ====================================================================

// ====================================================================
// ====================================================================
// CONSTRUCTOR & DESTRUCTOR

SceneParser_v2::SceneParser_v2(const char* filename) {

    // initialize some reasonable default values
    group = NULL;
    camera = NULL;
    background_color = Vec3f(0.5, 0.5, 0.5);
    ambient_light = Vec3f(0, 0, 0);
    num_lights = 0;
    lights = NULL;
    num_materials = 0;
    materials = NULL;
    current_material = NULL;

    // parse the file
    assert(filename != NULL);
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".txt"));
    file = fopen(filename, "r");
    assert(file != NULL);
    parseFile();
    fclose(file);
    file = NULL;

    // if no lights are specified, set ambient light to white 
    // (do solid color ray casting)
    if (num_lights == 0) {
        printf("WARNING:  No lights specified\n");
        ambient_light = Vec3f(1, 1, 1);
    }
}

SceneParser_v2::~SceneParser_v2() {
    if (group != NULL)
        delete group;
    if (camera != NULL)
        delete camera;
    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser_v2::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "OrthographicCamera")) {
            parseOrthographicCamera();
        }
        else if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Lights")) {
            parseLights();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v2::parseOrthographicCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "size"));
    float size = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    camera = new OrthographicCamera(center, direction, up, size);
}


void SceneParser_v2::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token); assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

void SceneParser_v2::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVec3f();
        }
        else if (!strcmp(token, "ambientLight")) {
            ambient_light = readVec3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v2::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light * [num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (!strcmp(token, "DirectionalLight")) {
            lights[count] = parseDirectionalLight();
        }
        else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Light* SceneParser_v2::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "color"));
    Vec3f color = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

// ====================================================================
// ====================================================================

void SceneParser_v2::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material * [num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if ((!strcmp(token, "PhongMaterial_v1")) || !strcmp(token, "Material")) {
            materials[count] = parseMaterial();
        }
        else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}


Material* SceneParser_v2::parseMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3f diffuseColor(1, 1, 1);
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "diffuseColor")) {
            diffuseColor = readVec3f();
        }
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new Material(diffuseColor);
    return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser_v2::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else if (!strcmp(token, "Plane")) {
        answer = (Object3D*)parsePlane();
    }
    else if (!strcmp(token, "Triangle")) {
        answer = (Object3D*)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D*)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform")) {
        answer = (Object3D*)parseTransform();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser_v2::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    Group* answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token); assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser_v2::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Sphere(center, radius, current_material);
}


Plane* SceneParser_v2::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "normal"));
    Vec3f normal = readVec3f();
    getToken(token); assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Plane(normal, offset, current_material);
}


Triangle* SceneParser_v2::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vec3f v0 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vec3f v1 = readVec3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vec3f v2 = readVec3f();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Triangle(v0, v1, v2, current_material);
}


Group* SceneParser_v2::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token); assert(!strcmp(token, "}"));
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    // read it once, get counts
    std::string path = "./resource/mesh/";
    std::string fname = filename;
    fname = path + fname;
    FILE* file = fopen((fname).c_str(), "r");
    assert(file != NULL);
    int vcount = 0; int fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            vcount++;
        }
        else if (c == 'f') {
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            fcount++;
        } // otherwise, must be whitespace
    }
    fclose(file);
    // make arrays
    Vec3f* verts = new Vec3f[vcount];
    Group* answer = new Group(fcount);
    // read it again, save it
    file = fopen(fname.c_str(), "r");
    assert(file != NULL);
    int new_vcount = 0; int new_fcount = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        else if (c == 'v') {
            assert(new_fcount == 0); float v0, v1, v2;
            fscanf(file, "%f %f %f", &v0, &v1, &v2);
            verts[new_vcount] = Vec3f(v0, v1, v2);
            new_vcount++;
        }
        else if (c == 'f') {
            assert(vcount == new_vcount);
            int f0, f1, f2;
            fscanf(file, "%d %d %d", &f0, &f1, &f2);
            // indexed starting at 1...
            assert(f0 > 0 && f0 <= vcount);
            assert(f1 > 0 && f1 <= vcount);
            assert(f2 > 0 && f2 <= vcount);
            assert(current_material != NULL);
            Triangle* t = new Triangle(verts[f0 - 1], verts[f1 - 1], verts[f2 - 1], current_material);
            answer->addObject(new_fcount, t);
            new_fcount++;
        } // otherwise, must be whitespace
    }
    delete[] verts;
    assert(fcount == new_fcount);
    assert(vcount == new_vcount);
    fclose(file);
    return answer;
}


Transform* SceneParser_v2::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix matrix; matrix.SetToIdentity();
    Object3D* object = NULL;
    getToken(token); assert(!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);
    while (1) {
        if (!strcmp(token, "Scale")) {
            matrix *= Matrix::MakeScale(readVec3f());
        }
        else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix *= Matrix::MakeScale(Vec3f(s, s, s));
        }
        else if (!strcmp(token, "Translate")) {
            matrix *= Matrix::MakeTranslation(readVec3f());
        }
        else if (!strcmp(token, "XRotate")) {
            matrix *= Matrix::MakeXRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate")) {
            matrix *= Matrix::MakeYRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate")) {
            matrix *= Matrix::MakeZRotation(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate")) {
            getToken(token); assert(!strcmp(token, "{"));
            Vec3f axis = readVec3f();
            float degrees = readFloat();
            matrix *= Matrix::MakeAxisRotation(axis, DegreesToRadians(degrees));
            getToken(token); assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix")) {
            Matrix matrix2; matrix2.SetToIdentity();
            getToken(token); assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2.Set(i, j, v);
                }
            }
            getToken(token); assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        }
        else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }
    assert(object != NULL);
    getToken(token); assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}


// ====================================================================
// ====================================================================

int SceneParser_v2::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != NULL);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3f SceneParser_v2::readVec3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vec3f\n");
        assert(0);
    }
    return Vec3f(x, y, z);
}


Vec2f SceneParser_v2::readVec2f() {
    float u, v;
    int count = fscanf(file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vec2f(u, v);
}


float SceneParser_v2::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser_v2::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}

#endif
#pragma endregion

#pragma region V1
#if (RTVersion==1)
// ====================================================================
// ====================================================================

SceneParser_v1::SceneParser_v1(const char* filename) {

    // initialize some reasonable default values
    group = NULL;
    camera = NULL;
    background_color = Vec3f(0.5, 0.5, 0.5);
    num_materials = 0;
    materials = NULL;
    current_material = NULL;

    // parse the file
    assert(filename != NULL);
    const char* ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".txt"));
    file = fopen(filename, "r");
    assert(file != NULL);
    parseFile();
    fclose(file);
    file = NULL;
}

SceneParser_v1::~SceneParser_v1() {
    if (group != NULL)
        delete group;
    if (camera != NULL)
        delete camera;
    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
}

// ====================================================================
// ====================================================================

void SceneParser_v1::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "OrthographicCamera")) {
            parseOrthographicCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v1::parseOrthographicCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "direction"));
    Vec3f direction = readVec3f();
    getToken(token); assert(!strcmp(token, "up"));
    Vec3f up = readVec3f();
    getToken(token); assert(!strcmp(token, "size"));
    float size = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    camera = new OrthographicCamera(center, direction, up, size);
}

void SceneParser_v1::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVec3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser_v1::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material * [num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "PhongMaterial_v1")) {
            materials[count] = parseMaterial();
        }
        else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token); assert(!strcmp(token, "}"));
}

Material* SceneParser_v1::parseMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vec3f diffuseColor(1, 1, 1);
    getToken(token); assert(!strcmp(token, "{"));
    while (1) {
        getToken(token);
        if (!strcmp(token, "diffuseColor")) {
            diffuseColor = readVec3f();
        }
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new Material(diffuseColor);
    return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser_v1::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser_v1::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token); assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    Group* answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token); assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser_v1::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token); assert(!strcmp(token, "{"));
    getToken(token); assert(!strcmp(token, "center"));
    Vec3f center = readVec3f();
    getToken(token); assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token); assert(!strcmp(token, "}"));
    assert(current_material != NULL);
    return new Sphere(center, radius, current_material);
}

// ====================================================================
// ====================================================================

int SceneParser_v1::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != NULL);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vec3f SceneParser_v1::readVec3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vec3f\n");
        assert(0);
    }
    return Vec3f(x, y, z);
}


Vec2f SceneParser_v1::readVec2f() {
    float u, v;
    int count = fscanf(file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vec2f(u, v);
}


float SceneParser_v1::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser_v1::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

#endif
#pragma endregion
#pragma endregion
#endif