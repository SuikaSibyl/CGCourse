#pragma once

#include "Forcefield.h"
#include "Particle.h"

class Integrator
{
public:
	virtual void Update(Particle* particle, ForceField* forcefield, float t, float dt) = 0;

	virtual Vec3f getColor()
	{
		return Vec3f();
	}
};

class EulerIntegrator :public Integrator
{
public:
	virtual void Update(Particle* particle, ForceField* forcefield, float t, float dt) override
	{
		Vec3f acceleration = forcefield->getAcceleration(particle->getPosition(), particle->getMass(), t);
		particle->setPosition(particle->getPosition() + particle->getVelocity() * dt);
		particle->setVelocity(particle->getVelocity() + acceleration * dt);
	}

	virtual Vec3f getColor()
	{
		return Vec3f(1, 0, 0);
	}
};

class MidpointIntegrator :public Integrator
{
public:
	virtual void Update(Particle* particle, ForceField* forcefield, float t, float dt) override
	{
		// particle : Pn Vn
		Vec3f acceleration1 = forcefield->getAcceleration(particle->getPosition(), particle->getMass(), t);
		Vec3f pm = particle->getPosition() + particle->getVelocity() * (dt / 2);
		Vec3f vm = particle->getVelocity() + acceleration1 * (dt / 2);

		particle->setPosition(particle->getPosition() + vm * dt);
		Vec3f acceleration = forcefield->getAcceleration(pm, particle->getMass(), t + dt / 2);
		particle->setVelocity(particle->getVelocity() + acceleration * dt);
	}

	virtual Vec3f getColor()
	{
		return Vec3f(0, 1, 0);
	}
};

class RungeKuttaIntegrator :public Integrator
{
public:
	virtual void Update(Particle* particle, ForceField* forcefield, float t, float dt) override
	{
		Vec3f pos1 = particle->getPosition();
		Vec3f vel1 = particle->getVelocity();
		Vec3f acc1 = forcefield->getAcceleration(pos1, particle->getMass(), t);

		Vec3f pos2 = particle->getPosition() + dt / 2 * vel1;
		Vec3f vel2 = particle->getVelocity() + dt / 2 * acc1;
		Vec3f acc2 = forcefield->getAcceleration(pos2, particle->getMass(), t + dt / 2);

		Vec3f pos3 = particle->getPosition() + dt / 2 * vel2;
		Vec3f vel3 = particle->getVelocity() + dt / 2 * acc2;
		Vec3f acc3 = forcefield->getAcceleration(pos3, particle->getMass(), t + dt / 2);

		Vec3f pos4 = particle->getPosition() + dt * vel3;
		Vec3f vel4 = particle->getVelocity() + dt * acc3;
		Vec3f acc4 = forcefield->getAcceleration(pos4, particle->getMass(), t + dt);

		Vec3f position_offset = (dt / 6.) * (vel1 + 2 * vel2 + 2 * vel3 + vel4);
		Vec3f velocity_offset = (dt / 6.) * (acc1 + 2 * acc2 + 2 * acc3 + acc4);
		particle->setPosition(particle->getPosition() + position_offset);
		particle->setVelocity(particle->getVelocity() + velocity_offset);
	}

	virtual Vec3f getColor()
	{
		return Vec3f(0, 0, 1);
	}
};