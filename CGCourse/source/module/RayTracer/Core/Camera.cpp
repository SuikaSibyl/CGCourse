#include "Camera.h"
#include <limits>
#include "../VersionControl.h"

// Included files for OpenGL Rendering
#include<windows.h>
#include <GL/gl.h>
#include <OpenGL/ThirdParty/glut.h>
#include <matrix.h>

#define LARGE_DISTANCE 999

// ==============================================
// Orthographic Camera
// ==============================================

OrthographicCamera::OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size)
	:size(size)
	,center(center)
{
	this->direction = direction;
	this->up = up;
	RefreshCamera();
}

void OrthographicCamera::RefreshCamera()
{
	direction.Normalize();
	up.Normalize();

	this->direction = direction;
	Vec3f::Cross3(this->horizontal, direction, up);
	Vec3f::Cross3(this->up, this->horizontal, direction);

	this->horizontal *= size;
	this->up *= size;
	leftbottom = this->center - 0.5 * this->horizontal - 0.5 * this->up;
}

Ray OrthographicCamera::generateRay(Vec2f point)
{
	Vec3f orig = leftbottom
		+ point.x() * this->horizontal
		+ point.y() * this->up;
	return Ray(orig, direction);
}

float OrthographicCamera::getTMin() const
{
	return numeric_limits<float>::lowest();
}

void OrthographicCamera::glInit(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w > h) glOrtho(-size / 2.0, size / 2.0, -size * (float)h / (float)w / 2.0, size * (float)h / (float)w / 2.0, 0.5, 40.0);
	else glOrtho(-size * (float)w / (float)h / 2.0, size * (float)w / (float)h / 2.0, -size / 2.0, size / 2.0, 0.5, 40.0);
}

void OrthographicCamera::glPlaceCamera(void) {
	gluLookAt(center.x(), center.y(), center.z(), center.x() + direction.x(), center.y() + direction.y(), center.z() + direction.z(), up.x(), up.y(), up.z());
}

// dollyCamera: Move camera along the direction vector 
// ==================================================================== 
void OrthographicCamera::dollyCamera(float dist) {
	center += direction * dist;
	RefreshCamera();
}

// ==================================================================== 
// truckCamera: Translate camera perpendicular to the direction vector 
// ==================================================================== 
void OrthographicCamera::truckCamera(float dx, float dy) {
	center += horizontal * dx + up * dy;
	RefreshCamera();
}
// ==================================================================== 
// rotateCamera: Rotate around the up and horizontal vectors 
// ====================================================================
void OrthographicCamera::rotateCamera(float rx, float ry) {

	//Vec3f horizontal; Vec3f::Cross3(this->horizontal, direction, up);
	//horizontal.Normalize();

	// Don't let the model flip upside-down (There is a singularity 
	// at the poles when 'up' and 'direction' are aligned) 
	float tiltAngle = acos(up.Dot3(direction));
	if (tiltAngle - ry > 3.13)
		ry = tiltAngle - 3.13;
	else if (tiltAngle - ry < 0.01)
		ry = tiltAngle - 0.01;
	Matrix rotMat = Matrix::MakeAxisRotation(up, rx);
	rotMat *= Matrix::MakeAxisRotation(horizontal, ry);
	rotMat.Transform(center);
	rotMat.TransformDirection(direction);

	RefreshCamera();
}

// ==============================================
// Perspective Camera
// ==============================================
// ==================================================================== 
// Create a perspective camera with the appropriate dimensions that 
// crops or stretches in the x-dimension as necessary 
// ==================================================================== 
void PerspectiveCamera::glInit(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angle * 180.0 / 3.14159, (float)w / float(h), 0.5, 40.0);
}

// ==================================================================== 
// Place a perspective camera within an OpenGL scene 
// ==================================================================== 
void PerspectiveCamera::glPlaceCamera(void) {
	gluLookAt(center.x(), center.y(), center.z(), center.x() + direction.x(), center.y() + direction.y(), center.z() + direction.z(), up.x(), up.y(), up.z());
}

// ==================================================================== 
// dollyCamera: Move camera along the direction vector 
// ==================================================================== 
void PerspectiveCamera::dollyCamera(float dist) {
	center += direction * dist;
	RefreshCamera();
} 
// ==================================================================== 
// truckCamera: Translate camera perpendicular to the direction vector 
// ==================================================================== 
void PerspectiveCamera::truckCamera(float dx, float dy) {
	center += horizontal * dx + up * dy;
	RefreshCamera();
}
// ==================================================================== 
// rotateCamera: Rotate around the up and horizontal vectors 
// ==================================================================== 
void PerspectiveCamera::rotateCamera(float rx, float ry) {
	// Don't let the model flip upside-down (There is a singularity 
	// at the poles when 'up' and 'direction' are aligned) 
	float tiltAngle = acos(up.Dot3(direction));
	if (tiltAngle - ry > 3.13)
		ry = tiltAngle - 3.13;
	else if (tiltAngle - ry < 0.01)
		ry = tiltAngle - 0.01;
	Matrix rotMat = Matrix::MakeAxisRotation(up, rx);
	rotMat *= Matrix::MakeAxisRotation(horizontal, ry);
	rotMat.Transform(center);
	rotMat.TransformDirection(direction);
	direction.Normalize();
	RefreshCamera();
}

PerspectiveCamera::PerspectiveCamera(Vec3f center, Vec3f& direction, Vec3f& up, float angle)
	:center(center), angle(angle)
{
	this->direction = direction;
	this->up = up;
	RefreshCamera();
}

void PerspectiveCamera::RefreshCamera()
{
	direction.Normalize();
	up.Normalize();

	this->direction = direction;
	Vec3f::Cross3(this->horizontal, direction, up);
	Vec3f::Cross3(this->up, this->horizontal, direction);

	width = 2 * tanf(angle / 2);
	leftbottom = this->center + this->direction
		- 0.5 * width * this->horizontal
		- 0.5 * width * this->up;
}

Ray PerspectiveCamera::generateRay(Vec2f point)
{
	Vec3f direction = leftbottom - center
		+ point.x() * width * this->horizontal
		+ point.y() * width * this->up;
	direction.Normalize();
	return Ray(center, direction);
}

float PerspectiveCamera::getTMin() const
{
	return 0;
}