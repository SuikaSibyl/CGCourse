#pragma once

#include <vector>
#include "Particle.h"
#include "Random.h"

#include <windows.h>
#include <GL/gl.h>
#include "../OpenGL/ThirdParty/glut.h"

class Generator
{
public:
	virtual int numNewParticles(float time, float deltatime)
	{
		return 0;
	}

	virtual Particle* Generate(float time, int i)
	{
		return nullptr;
	}

	void Restart()
	{

	}

	void Paint() const
	{
		//for (int i = 0; i < Particles.size(); i++)
		//{
		//	Vec3f position = Particles[i].getPosition();
		//	Vec3f color = Particles[i].getColor();
		//	glPointSize(3);
		//	glBegin(GL_POINTS);
		//	glColor3f(color.x(), color.y(), color.z());
		//	glVertex3f(position.x(), position.y(), position.z());
		//	glEnd();
		//}
	}

	void SetColors(Vec3f color, Vec3f dead_color, float color_randomness)
	{
		Color = color;
		DeadColor = dead_color;
		ColorRandomness = color_randomness;
	}

	void SetLifespan(float lifespan, float lifespan_randomness, int desired_num_particles)
	{
		Lifespan = lifespan;
		Lifespan_randomness = lifespan_randomness;
		Desired_num_particles = desired_num_particles;
	}

	void SetMass(float mass, float mass_randomness)
	{
		Mass = mass;
		Mass_randomness = mass_randomness;
	}

protected:
	Vec3f Color;
	Vec3f DeadColor;
	float ColorRandomness;

	float Lifespan;
	float Lifespan_randomness;
	int Desired_num_particles;

	float Mass;
	float Mass_randomness;

	Random random;

	std::vector<Particle> Particles;
};

class HoseGenerator :public Generator
{
public:
	HoseGenerator(Vec3f position, float position_randomness, Vec3f velocity, float velocity_randomness)
		: position(position)
		, velocity(velocity)
		, position_randomness(position_randomness)
		, velocity_randomness(velocity_randomness)
	{

	}

	virtual int numNewParticles(float time, float deltatime)
	{
		return ceil(deltatime * Desired_num_particles / Lifespan);
	}

	virtual Particle* Generate(float time, int i)
	{
		Vec3f p = position + position_randomness*random.randomVector();
		Vec3f v = velocity + velocity_randomness * random.randomVector();
		return new Particle(p,v, Color, DeadColor, Mass, Lifespan);
	}

private:
	Vec3f position;
	float position_randomness;
	Vec3f velocity;
	float velocity_randomness;
};

class RingGenerator :public Generator
{
public:
	RingGenerator(float position_randomness, Vec3f velocity, float velocity_randomness)
		: velocity(velocity)
		, position_randomness(position_randomness)
		, velocity_randomness(velocity_randomness)
	{

	}

	virtual int numNewParticles(float time, float deltatime)
	{
		std::cout << time << std::endl;
		return time * 20;
	}

	virtual Particle* Generate(float time, int i)
	{
		float radius = time;
		float theta = 2 * 3.1415926 * random.next();
		Vec3f position(radius * cos(theta), 0, radius * sin(theta));
		position += position_randomness * random.randomVector();
		Vec3f v = velocity + velocity_randomness * random.randomVector();
		return new Particle(position, v, Color, DeadColor, Mass, Lifespan);
	}


private:
	float position_randomness;
	Vec3f velocity;
	float velocity_randomness;
};