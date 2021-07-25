#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include "../VersionControl.h"

// ====================================================================
// ====================================================================

// You will extend this class in later assignments


class Hit;
class Ray;

class Material {

public:

  // CONSTRUCTORS & DESTRUCTOR
  Material(const Vec3f &d_color) { diffuseColor = d_color; }
  virtual ~Material() {}

  // ACCESSORS
  virtual Vec3f getDiffuseColor() const { return diffuseColor; }

#if (RTVersion>2)
  virtual Vec3f Shade (const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
	  const Vec3f& lightColor) const = 0;

  virtual void glSetMaterial(void) const = 0;
#endif
protected:

  // REPRESENTATION
  Vec3f diffuseColor;
  
};

// ====================================================================
// ====================================================================

class PhongMaterial : public Material
{
public:
	PhongMaterial(const Vec3f& diffuseColor, const Vec3f& specularColor, const float& exponent)
		: Material(diffuseColor)
		, specularColor(specularColor)
		, exponent(exponent)
	{

	}

	virtual Vec3f getSpecularColor() const { return specularColor; }

	virtual Vec3f Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
		const Vec3f& lightColor) const;

	virtual void glSetMaterial(void) const;
private:
	// REPRESENTATION
	Vec3f specularColor;
	float exponent;
};

#endif
