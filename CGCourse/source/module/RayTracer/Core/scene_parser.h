#ifndef _SceneParser_H_
#define _SceneParser_H_

#include "vectors.h"
#include <assert.h>

#pragma warning(disable:4996)

class Camera;
class Light;
class Material;
class Object3D;
class Group;
class Sphere;
class Plane;
class Triangle;
class Transform;

#define MAX_PARSER_TOKEN_LENGTH 100

#define MAX_PARSER_TOKEN_LENGTH 100

// ====================================================================
// ====================================================================

class SceneParser_v4 {

public:

    // CONSTRUCTOR & DESTRUCTOR
    SceneParser_v4(const char* filename);
    ~SceneParser_v4();

    // ACCESSORS
    Camera* getCamera() const { return camera; }
    Vec3f getBackgroundColor() const { return background_color; }
    Vec3f getAmbientLight() const { return ambient_light; }
    int getNumLights() const { return num_lights; }
    Light* getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }
    int getNumMaterials() const { return num_materials; }
    Material* getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }
    Group* getGroup() const { return group; }

private:

    SceneParser_v4() { assert(0); } // don't use

    // PARSING
    void parseFile();
    void parseOrthographicCamera();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light* parseDirectionalLight();
    Light* parsePointLight();
    void parseMaterials();
    Material* parsePhongMaterial();

    Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    Sphere* parseSphere();
    Plane* parsePlane();
    Triangle* parseTriangle();
    Group* parseTriangleMesh();
    Transform* parseTransform();

    // HELPER FUNCTIONS
    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
    Vec3f readVec3f();
    Vec2f readVec2f();
    float readFloat();
    int readInt();

    // ==============
    // REPRESENTATION
    FILE* file;
    Camera* camera;
    Vec3f background_color;
    Vec3f ambient_light;
    int num_lights;
    Light** lights;
    int num_materials;
    Material** materials;
    Material* current_material;
    Group* group;
};

// ====================================================================
// ====================================================================


#pragma region DEPRECATED
// ====================================================================
// ====================================================================

class SceneParser_v3 {

public:

    // CONSTRUCTOR & DESTRUCTOR
    SceneParser_v3(const char* filename);
    ~SceneParser_v3();

    // ACCESSORS
    Camera* getCamera() const { return camera; }
    Vec3f getBackgroundColor() const { return background_color; }
    Vec3f getAmbientLight() const { return ambient_light; }
    int getNumLights() const { return num_lights; }
    Light* getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }
    int getNumMaterials() const { return num_materials; }
    Material* getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }
    Group* getGroup() const { return group; }

private:

    SceneParser_v3() { assert(0); } // don't use

    // PARSING
    void parseFile();
    void parseOrthographicCamera();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light* parseDirectionalLight();
    void parseMaterials();
    Material* parsePhongMaterial();

    Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    Sphere* parseSphere();
    Plane* parsePlane();
    Triangle* parseTriangle();
    Group* parseTriangleMesh();
    Transform* parseTransform();

    // HELPER FUNCTIONS
    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
    Vec3f readVec3f();
    Vec2f readVec2f();
    float readFloat();
    int readInt();

    // ==============
    // REPRESENTATION
    FILE* file;
    Camera* camera;
    Vec3f background_color;
    Vec3f ambient_light;
    int num_lights;
    Light** lights;
    int num_materials;
    Material** materials;
    Material* current_material;
    Group* group;
};

// ====================================================================
// ====================================================================

// ====================================================================
// ====================================================================

class SceneParser_v2 {

public:

    // CONSTRUCTOR & DESTRUCTOR
    SceneParser_v2(const char* filename);
    ~SceneParser_v2();

    // ACCESSORS
    Camera* getCamera() const { return camera; }
    Vec3f getBackgroundColor() const { return background_color; }
    Vec3f getAmbientLight() const { return ambient_light; }
    int getNumLights() const { return num_lights; }
    Light* getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }
    int getNumMaterials() const { return num_materials; }
    Material* getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }
    Group* getGroup() const { return group; }

private:

    SceneParser_v2() { assert(0); } // don't use

    // PARSING
    void parseFile();
    void parseOrthographicCamera();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light* parseDirectionalLight();
    void parseMaterials();
    Material* parseMaterial();

    Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    Sphere* parseSphere();
    Plane* parsePlane();
    Triangle* parseTriangle();
    Group* parseTriangleMesh();
    Transform* parseTransform();

    // HELPER FUNCTIONS
    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
    Vec3f readVec3f();
    Vec2f readVec2f();
    float readFloat();
    int readInt();

    // ==============
    // REPRESENTATION
    FILE* file;
    Camera* camera;
    Vec3f background_color;
    Vec3f ambient_light;
    int num_lights;
    Light** lights;
    int num_materials;
    Material** materials;
    Material* current_material;
    Group* group;
};

// ====================================================================
// ====================================================================

class SceneParser_v1 {

public:

  // CONSTRUCTOR & DESTRUCTOR
  SceneParser_v1(const char *filename);
  ~SceneParser_v1();

  // ACCESSORS
  Camera* getCamera() const { return camera; }
  Vec3f getBackgroundColor() const { return background_color; }
  int getNumMaterials() const { return num_materials; }
  Material* getMaterial(int i) const { 
    assert(i >= 0 && i < num_materials); 
    return materials[i]; }  
  Group* getGroup() const { return group; }

private:

  SceneParser_v1() { assert(0); } // don't use

  // PARSING
  void parseFile();
  void parseOrthographicCamera();
  void parseBackground();
  void parseMaterials();
  Material* parseMaterial();

  Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
  Group* parseGroup();
  Sphere* parseSphere();

  // HELPER FUNCTIONS
  int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
  Vec3f readVec3f();
  Vec2f readVec2f();
  float readFloat();
  int readInt();

  // ==============
  // REPRESENTATION
  FILE *file;
  Camera *camera;
  Vec3f background_color;
  int num_materials;
  Material **materials;
  Material *current_material;
  Group *group;
};

// ====================================================================
// ====================================================================

#pragma endregion

#endif
