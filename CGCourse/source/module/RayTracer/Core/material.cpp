#include "material.h"
#include "hit.h"
#include "ray.h"
#include <OpenGL/Core/GLCanvas.h> 

#include <Windows.h>
#include <gl/GL.h>
#include <OpenGL/ThirdParty/glut.h>

Vec3f Material::Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
    const Vec3f& lightColor) const
{
    // Shading
    Vec3f radiance(0, 0, 0);
    Vec3f albedo = hit.getMaterial()->getDiffuseColor();
    Vec3f normal = hit.getNormal();

    // Diffuse
    Vec3f diffuse(0, 0, 0);
    Vec3f::Mult(diffuse, albedo, lightColor);
    float cosine = normal.Dot3(dirToLight);
    diffuse *= max(normal.Dot3(dirToLight), 0.0f);
    radiance += diffuse;

    return radiance;
}

void Material::glSetMaterial(void) const
{
    GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
    GLfloat diffuse[4] = {
      getDiffuseColor().r(),
      getDiffuseColor().g(),
      getDiffuseColor().b(),
      1.0 };

#if !SPECULAR_FIX 

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);

#else

    // OPTIONAL: 3 pass rendering to fix the specular highlight 
    // artifact for small specular exponents (wide specular lobe)

    if (SPECULAR_FIX_WHICH_PASS == 0) {
        // First pass, draw only the specular highlights
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

    }
    else if (SPECULAR_FIX_WHICH_PASS == 1) {
        // Second pass, compute normal dot light 
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }
    else {
        // Third pass, add ambient & diffuse terms
        assert(SPECULAR_FIX_WHICH_PASS == 2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }

#endif
}

#pragma region V2
#if (RTVersion>=4)
PhongMaterial_v2::PhongMaterial_v2(
    const Vec3f& diffuseColor,
    const Vec3f& specularColor,
    float exponent,
    const Vec3f& reflectiveColor,
    const Vec3f& transparentColor,
    float indexOfRefraction)
    : Material(diffuseColor)
    , specularColor(specularColor)
    , reflectiveColor(reflectiveColor)
    , transparentColor(transparentColor)
    , exponent(exponent)
    , indexOfRefraction(indexOfRefraction)
{

}

Vec3f reflect(Vec3f input, Vec3f normal)
{
    Vec3f tmp = normal;
    float proj = tmp.Dot3(input);
    return input - 2.0 * normal * proj;
}

Vec3f PhongMaterial_v2::Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
    const Vec3f& lightColor) const
{
    // Shading
    Vec3f radiance(0, 0, 0);
    Vec3f albedo = getDiffuseColor();
    Vec3f normal = hit.getNormal();

    // Diffuse
    Vec3f diffuse(0, 0, 0);
    Vec3f::Mult(diffuse, albedo, lightColor);
    float cosine = normal.Dot3(dirToLight);
    diffuse *= max(normal.Dot3(dirToLight), 0.0f);
    radiance += diffuse;

    // Specular
    Vec3f viewDir = ray.getDirection();
    viewDir.Normalize();
    Vec3f half = -1. * viewDir + dirToLight;
    half.Normalize();
    Vec3f reflected = reflect(-1.f * dirToLight, normal);
    reflected.Normalize();

    //float NdH = max(half.Dot3(normal), 0.0);
    float NdL = normal.Dot3(dirToLight);
    //float NdH = max(reflected.Dot3(-1.f * viewDir), 0.0);
    float NdH = max(reflected.Dot3(-1.f * viewDir), 0.0);
    float spec = pow(NdH, exponent);
    Vec3f specular = spec * lightColor * specularColor;
    radiance += specular;

    return radiance;
}

// ====================================================================
// OPTIONAL: 3 pass rendering to fix the specular highlight 
// artifact for small specular exponents (wide specular lobe)
// ====================================================================

// include glCanvas.h to access the preprocessor variable SPECULAR_FIX

#ifdef SPECULAR_FIX
// OPTIONAL:  global variable allows (hacky) communication 
// with glCanvas::display
extern int SPECULAR_FIX_WHICH_PASS;
#endif

// ====================================================================
// Set the OpenGL parameters to render with the given material
// attributes.
// ====================================================================

void PhongMaterial_v2::glSetMaterial(void) const {

    GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
    GLfloat specular[4] = {
      getSpecularColor().r(),
      getSpecularColor().g(),
      getSpecularColor().b(),
      1.0 };
    GLfloat diffuse[4] = {
      getDiffuseColor().r(),
      getDiffuseColor().g(),
      getDiffuseColor().b(),
      1.0 };

    // NOTE: GL uses the Blinn Torrance version of Phong...      
    float glexponent = exponent;
    if (glexponent < 0) glexponent = 0;
    if (glexponent > 128) glexponent = 128;


#if !SPECULAR_FIX 

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

#else

    // OPTIONAL: 3 pass rendering to fix the specular highlight 
    // artifact for small specular exponents (wide specular lobe)

    if (SPECULAR_FIX_WHICH_PASS == 0) {
        // First pass, draw only the specular highlights
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

    }
    else if (SPECULAR_FIX_WHICH_PASS == 1) {
        // Second pass, compute normal dot light 
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }
    else {
        // Third pass, add ambient & diffuse terms
        assert(SPECULAR_FIX_WHICH_PASS == 2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }

#endif
}

#endif // (RTVersion==4)
#pragma endregion


#pragma region DEPRECATED
#if (RTVersion <4)
#if (RTVersion>2)
Vec3f reflect(Vec3f input, Vec3f normal)
{
    Vec3f tmp = normal;
    float proj = tmp.Dot3(input);
    return input - 2.0 * normal * proj;
}

Vec3f PhongMaterial_v1::Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
    const Vec3f& lightColor) const
{
    // Shading
    Vec3f radiance(0, 0, 0);
    Vec3f albedo = hit.getMaterial()->getDiffuseColor();
    Vec3f normal = hit.getNormal();

    // Diffuse
    Vec3f diffuse(0, 0, 0);
    Vec3f::Mult(diffuse, albedo, lightColor);
    float cosine = normal.Dot3(dirToLight);
    diffuse *= max(normal.Dot3(dirToLight), 0.0f);
    radiance += diffuse;

    // Specular
    Vec3f viewDir = ray.getDirection();
    viewDir.Normalize();
    Vec3f half = -1. * viewDir + dirToLight;
    half.Normalize();
    Vec3f reflected = reflect(-1.f * dirToLight, normal);
    reflected.Normalize();

    //float NdH = max(half.Dot3(normal), 0.0);
    float NdL = normal.Dot3(dirToLight);
    //float NdH = max(reflected.Dot3(-1.f * viewDir), 0.0);
    float NdH = max(reflected.Dot3(-1.f * viewDir), 0.0);
    float spec = pow(NdH, exponent);
    Vec3f specular = spec * lightColor * specularColor;
    radiance += specular;

    return radiance;
}

// ====================================================================
// OPTIONAL: 3 pass rendering to fix the specular highlight 
// artifact for small specular exponents (wide specular lobe)
// ====================================================================

// include glCanvas.h to access the preprocessor variable SPECULAR_FIX

#ifdef SPECULAR_FIX
// OPTIONAL:  global variable allows (hacky) communication 
// with glCanvas::display
extern int SPECULAR_FIX_WHICH_PASS;
#endif

// ====================================================================
// Set the OpenGL parameters to render with the given material
// attributes.
// ====================================================================

void PhongMaterial_v1::glSetMaterial(void) const {

    GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
    GLfloat specular[4] = {
      getSpecularColor().r(),
      getSpecularColor().g(),
      getSpecularColor().b(),
      1.0 };
    GLfloat diffuse[4] = {
      getDiffuseColor().r(),
      getDiffuseColor().g(),
      getDiffuseColor().b(),
      1.0 };

    // NOTE: GL uses the Blinn Torrance version of Phong...      
    float glexponent = exponent;
    if (glexponent < 0) glexponent = 0;
    if (glexponent > 128) glexponent = 128;


#if !SPECULAR_FIX 

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

#else

    // OPTIONAL: 3 pass rendering to fix the specular highlight 
    // artifact for small specular exponents (wide specular lobe)

    if (SPECULAR_FIX_WHICH_PASS == 0) {
        // First pass, draw only the specular highlights
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

    }
    else if (SPECULAR_FIX_WHICH_PASS == 1) {
        // Second pass, compute normal dot light 
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }
    else {
        // Third pass, add ambient & diffuse terms
        assert(SPECULAR_FIX_WHICH_PASS == 2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
    }

#endif
}

// ====================================================================
// ====================================================================
#endif
#pragma endregion

#endif
#pragma endregion