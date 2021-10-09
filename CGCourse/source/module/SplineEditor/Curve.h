#pragma once

#include "Spline.h"
#include <vector>
#include <vectors.h>
#include <matrix.h>

#include <windows.h>
#include <GL/gl.h>
#include "ArgParser.h"
#include "../OpenGL/ThirdParty/glut.h"

float bezier_paras[] =
{
	-1,3,-3,1,
	3,-6,3,0,
	-3,3,0,0,
	1,0,0,0
};
Matrix B_bezier(bezier_paras);

float bspline_paras[] =
{
	-1. / 6,3. / 6,-3. / 6,1. / 6,
	3. / 6,-6. / 6,0,4. / 6,
	-3. / 6,3. / 6,3. / 6,1. / 6,
	1. / 6,0,0,0
};
Matrix B_bspline(bspline_paras);


class Curve :public Spline
{
public:
	Curve(int i) :n(i) {}
	// FOR VISUALIZATION
	virtual void Paint(ArgParser* args) override
	{
		glLineWidth(1);
		for (int i = 0; i < controlPoints.size() - 1; i++)
		{
			glBegin(GL_LINES);
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(controlPoints[i].x(), controlPoints[i].y(), controlPoints[i].z());
			glVertex3f(controlPoints[i + 1].x(), controlPoints[i + 1].y(), controlPoints[i + 1].z());
			glEnd();
		}

		DrawCurve(args->curve_tessellation);

		glPointSize(5);
		for (int i = 0; i < controlPoints.size(); i++)
		{
			glBegin(GL_POINTS);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(controlPoints[i].x(), controlPoints[i].y(), controlPoints[i].z());
			glEnd();
		}
	}

	int GetN() { return n; }

	virtual void DrawCurve(int tessellation) {}

	virtual void set(int i, const Vec3f v)
	{
		if (i >= controlPoints.size())
		{
			controlPoints.emplace_back(Vec3f());
		}
		controlPoints[i] = v;
	}
	// FOR CONTROL POINT PICKING
	virtual int getNumVertices() { return controlPoints.size(); }
	virtual Vec3f getVertex(int i) { return controlPoints[i]; }
	// FOR EDITING OPERATIONS
	virtual void moveControlPoint(int selectedPoint, float x, float y)
	{
		controlPoints[selectedPoint] = Vec3f(x, y, 0);
	}
	virtual void addControlPoint(int selectedPoint, float x, float y)
	{
		std::vector<Vec3f>::iterator iter = controlPoints.begin();
		for (int i = 0; i < selectedPoint; i++) iter++;
		controlPoints.insert(iter, Vec3f(x, y, 0));
		n++;
	}

	virtual void deleteControlPoint(int selectedPoint)
	{
		if (selectedPoint < 4)
			return;
		std::vector<Vec3f>::iterator iter = controlPoints.begin();
		for (int i = 0; i < selectedPoint; i++) iter++;
		controlPoints.erase(iter);
		n--;
	}
	int n;
	virtual Vec3f GetInterpolation(float t) = 0;

protected:
	std::vector<Vec3f> controlPoints;
};

class BezierCurve :public Curve
{
public:
	BezierCurve(int i)
		:Curve(i - 1)
	{
		Matrix bspline = B_bspline;
		bspline.Inverse();
		bezier2bspline = B_bezier * bspline;
		bezier2bspline.Transpose();
	}

	Matrix bezier2bspline;
	
	virtual void OutputBSpline(FILE* file)
	{
		Vec4f x(controlPoints[0].x(), controlPoints[1].x(), controlPoints[2].x(), controlPoints[3].x());
		Vec4f y(controlPoints[0].y(), controlPoints[1].y(), controlPoints[2].y(), controlPoints[3].y());
		Vec4f z(controlPoints[0].z(), controlPoints[1].z(), controlPoints[2].z(), controlPoints[3].z());

		bezier2bspline.Transform(x);
		bezier2bspline.Transform(y);
		bezier2bspline.Transform(z);


		fprintf(file, "\nbspline\n");
		fprintf(file, "num_vertices %d\n", controlPoints.size());
		Vec3f points[] =
		{
			{x.x(),y.x(),z.x()},
			{x.y(),y.y(),z.y()},
			{x.z(),y.z(),z.z()},
			{x.w(),y.w(),z.w()},
		};
		for (int i = 0; i < controlPoints.size(); i++)
		{
			fprintf(file, "%f ", points[i].x());
			fprintf(file, "%f ", points[i].y());
			fprintf(file, "%f\n", points[i].z());
		}
	}

	virtual void OutputBezier(FILE* file)
	{
		fprintf(file, "\nbezier\n");
		fprintf(file, "num_vertices %d\n", controlPoints.size());
		for (int i = 0; i < controlPoints.size(); i++)
		{
			fprintf(file, "%f ", controlPoints[i].x());
			fprintf(file, "%f ", controlPoints[i].y());
			fprintf(file, "%f\n", controlPoints[i].z());
		}
	}

	virtual void DrawCurve(int tessellation)
	{
		glLineWidth(3);
		float step = 1.0 / (tessellation * n / 3);
		Vec3f previous = GetInterpolation(0);
		for (int i = 1; i <= tessellation * n / 3; i++)
		{
			Vec3f curr = GetInterpolation(i * step);
			glBegin(GL_LINES);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(previous.x(), previous.y(), previous.z());
			glVertex3f(curr.x(), curr.y(), curr.z());
			glEnd();
			previous = curr;
		}
	}

	float GetWeight(int i, float t)
	{
		int n = 3;
		int omi = n - i;
		float omt = 1 - t;
		float up = 1;
		float down = 1;
		for (int j = n; j > n - i; j--)
			up *= j;
		for (int j = i; j > 0; j--)
			down *= j;

		return up / down * pow(omt, omi) * pow(t, i);
	}

	virtual Vec3f GetInterpolation(float t)
	{
		float omt = 1 - t;
		Vec3f res;

		int k = n / 3;
		float kstep = 1. / k;
		int index = floor(t / kstep);
		if (t == 1) index--;
		float weight = t - index * kstep;
		weight = weight / kstep;

		for (int i = 0; i <= 3; i++)
		{
			res += GetWeight(i, weight) * controlPoints[3 * index + i];
		}
		return res;

		return res;
	}
};

class BSplineCurve :public Curve
{
public:

	BSplineCurve(int i)
		:Curve(i - 1)
	{
		Matrix bezier = B_bezier;
		bezier.Inverse();
		bspline2bezier = B_bspline * bezier;
		bspline2bezier.Transpose();

	}

	virtual void DrawCurve(int tessellation)
	{
		glLineWidth(3);
		float step = 1.0 / (tessellation * (n - 2));
		Vec3f previous = GetInterpolation(0);
		for (int i = 1; i <= (tessellation * (n - 2)); i++)
		{
			if (i * step >= 1.0)
			{
				std::cout << "what";
			}
			Vec3f curr = GetInterpolation(i * step);
			glBegin(GL_LINES);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(previous.x(), previous.y(), previous.z());
			glVertex3f(curr.x(), curr.y(), curr.z());
			glEnd();
			previous = curr;
		}
	}

	float GetWeight(int i, float t)
	{
		switch (i)
		{
		case 0:
		{
			float omt = 1 - t;
			return omt * omt * omt / 6;
		}
			break;
		case 1:
		{
			return (3 * t * t * t - 6 * t * t + 4) / 6;
		}
		break;
		case 2:
		{
			return (-3 * t * t * t + 3 * t * t + 3 * t + 1) / 6;
		}
		break;
		case 3:
		{
			return (t * t * t) / 6;
		}
		break;
		default:
			break;
		}
		return -1;
	}

	virtual Vec3f GetInterpolation(float t)
	{
		int numline = n - 2;
		float step = 1. / numline;
		float index = floor(t / step);

		if (t == 1 && (t / step - index) < 0.99999)
		{
			index--;
		}
		float weight = t - index * step;
		weight = weight / step;

		Vec3f res;
		for (int i = 0; i <= 3; i++)
		{
			res += GetWeight(i, weight) * controlPoints[index + i];
		}
		return res;
	}
	Matrix bspline2bezier;
	virtual void OutputBezier(FILE* file)
	{
		Vec4f x(controlPoints[0].x(), controlPoints[1].x(), controlPoints[2].x(), controlPoints[3].x());
		Vec4f y(controlPoints[0].y(), controlPoints[1].y(), controlPoints[2].y(), controlPoints[3].y());
		Vec4f z(controlPoints[0].z(), controlPoints[1].z(), controlPoints[2].z(), controlPoints[3].z());

		bspline2bezier.Transform(x);
		bspline2bezier.Transform(y);
		bspline2bezier.Transform(z);


		fprintf(file, "\nbspline\n");
		fprintf(file, "num_vertices %d\n", controlPoints.size());
		Vec3f points[] =
		{
			{x.x(),y.x(),z.x()},
			{x.y(),y.y(),z.y()},
			{x.z(),y.z(),z.z()},
			{x.w(),y.w(),z.w()},
		};
		for (int i = 0; i < controlPoints.size(); i++)
		{
			fprintf(file, "%f ", points[i].x());
			fprintf(file, "%f ", points[i].y());
			fprintf(file, "%f\n", points[i].z());
		}
	}

	virtual void OutputBSpline(FILE* file)
	{
		fprintf(file, "\nbspline\n");
		fprintf(file, "num_vertices %d\n", controlPoints.size());
		for (int i = 0; i < controlPoints.size(); i++)
		{
			fprintf(file, "%f ", controlPoints[i].x());
			fprintf(file, "%f ", controlPoints[i].y());
			fprintf(file, "%f\n", controlPoints[i].z());
		}
	}
};