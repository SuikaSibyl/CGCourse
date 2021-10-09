#pragma once

#include <vectors.h>

class Film;

class Filter
{
public:
	Filter() = default;
	virtual ~Filter() = default;

	// Computes the appropriately filtered color for pixel (i,j) from the samples stored in the Film instance
	Vec3f getColor(int i, int j, Film* film);

	// Returns the weight for point (x + pixel center, y + pixel center)
	// In general, this function will have maximum value at the pixel center (when x=0 & y=0).
	virtual float getWeight(float x, float y) = 0;

	// Indicate which pixel samples might contribute to the final color
	// If the filter only relies on samples in the center pixel, this function returns zero.
	// If it relies on the center pixel and the 8 neighboring pixels (9 pixels total) this function returns 1, etc.
	// x : [i-supportRadius, i+supportRadius]
	// y : [j-supportRadius, j+supportRadius]
	virtual int getSupportRadius() = 0;
};

class BoxFilter :public Filter
{
public:
	BoxFilter(float radius) :Radius(radius) {}
	virtual ~BoxFilter() = default;

	virtual float getWeight(float x, float y) override;
	virtual int getSupportRadius() override { return ceil(Radius - 0.5); }

private:
	float Radius = 0;
};

class TentFilter :public Filter
{
public:
	TentFilter(float radius) :Radius(radius) {}
	virtual ~TentFilter() = default;

	virtual float getWeight(float x, float y) override;
	virtual int getSupportRadius() override { return ceil(Radius - 0.5); }

private:
	float Radius = 0;
};

class GaussianFilter :public Filter
{
public:
	GaussianFilter(float sigma) :Sigma(sigma) {}
	virtual ~GaussianFilter() = default;

	virtual float getWeight(float x, float y) override;
	virtual int getSupportRadius() override { return ceil(2 * Sigma - 0.5); }

private:
	float Sigma = 0;
};