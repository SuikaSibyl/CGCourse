#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"

// ====================================================================
// ====================================================================

// You will extend this class in later assignments

class Material {

public:

  // CONSTRUCTORS & DESTRUCTOR
  Material(const Vec3f &d_color) { diffuseColor = d_color; }
  virtual ~Material() {}

  // ACCESSORS
  virtual Vec3f getDiffuseColor() const { return diffuseColor; }

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

private:
	// REPRESENTATION
	Vec3f specularColor;
	float exponent;
};

#endif
