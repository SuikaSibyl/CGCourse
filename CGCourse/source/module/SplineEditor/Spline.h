#pragma once

#include <vectors.h>

class TriangleMesh;
class ArgParser;

class Spline
{
public:
	// FOR VISUALIZATION
	virtual void Paint(ArgParser* args) {}

	// FOR CONVERTING BETWEEN SPLINE TYPES
	virtual void OutputBezier(FILE* file) {}
	virtual void OutputBSpline(FILE* file) {}

	// FOR CONTROL POINT PICKING
	virtual int getNumVertices() { return 0; }
	virtual Vec3f getVertex(int i) { return Vec3f(); }

	// FOR EDITING OPERATIONS
	virtual void moveControlPoint(int selectedPoint, float x, float y) {}
	virtual void addControlPoint(int selectedPoint, float x, float y) {}
	virtual void deleteControlPoint(int selectedPoint) {}

	// FOR GENERATING TRIANGLES
	virtual TriangleMesh* OutputTriangles(ArgParser* args) { return nullptr; }
};