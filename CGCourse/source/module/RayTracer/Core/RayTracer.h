#pragma once
#include <LinearAlgebra/vectors.h>

class SceneParser;
class Ray;
class Hit;

// computes the radiance (color) along a ray.
class RayTracer
{
public:
	RayTracer(SceneParser* s, int max_bounces, float cutoff_weight, bool shadows);

private:
	// The main method of this class is traceRay that, 
	// given a ray, computes the color seen from the origin along the direction.
	// This computation is recursive for reflected or transparent materials.
	// We therefore need a stopping criterion to prevent infinite recursion.
	// traceRay takes as additional parameters the current number of bounces
	// (recursion depth) and a ray weight that indicates the percent contribution
	// of this ray to the final pixel color.
	
	// The corresponding maximum recursion depth and the cutoff ray weight are fields of RayTracer,
	// which are passed as command line arguments to the program.
	// Note that weight is a scalar that corresponds to the magnitude of the color vector.

	// To refract rays through transparent objects,
	// traceRay is also passed the indexOfRefraction (see below),
	// and returns the closest intersection in hit,
	// which is used to create the depth & normal visualizations.
	
	// You can test your code at this point with examples from previous assignments.

	Vec3f traceRay(Ray& ray, float tmin, int bounces, float weight,
		float indexOfRefraction, Hit& hit) const;
};