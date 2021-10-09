#include "RandomModule.h"
#include <iostream>

RandomModule::RandomModule()
    :randDev(), UFloatDistribution(0, 1)
{

}

float RandomModule::RandFloat01()
{
    return UFloatDistribution(randDev);
}

Vec3f RandomModule::RandVec2f0101()
{
    return std::move(Vec3f(RandFloat01(), RandFloat01(), 0));
}

Vec2f RandomModule::RandVec2f_0101()
{
    return std::move(Vec2f(RandFloat01(), RandFloat01()));
}