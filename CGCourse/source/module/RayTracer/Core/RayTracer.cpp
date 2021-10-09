#include "RayTracer.h"

#include "RayTracingStas.h"
#include <RayTracer/Primitives/Group.h>
#include <RayTracer/Core/light.h>
#include <RayTracer/Core/RayTree.h>
#include <limits>

static Vec3f black(0, 0, 0);
static Vec3f invalid(-1, -1, -1);
static Material* black_mat = new PhongMaterial(black, black, 0, black, black, 1);

bool visualize_grid = false;
int gridx = 0, gridy = 0, gridz = 0;
bool grid = false;

RayTracer::RayTracer(SceneParser_v6* s, int max_bounces, float cutoff_weight, bool shadows)
    :pSceneParser(s), mUseShadow(shadows), mMaxBounce(max_bounces), mCutoffweight(cutoff_weight)
{
    mShadeback = true;

    if (grid)
    {
        ScopeGrid = std::make_unique<Grid>(pSceneParser->getGroup()->getBoundingBox(), gridx, gridy, gridz);
        ScopeGrid->setBoundingBox(pSceneParser->getGroup()->getBoundingBox());
        pSceneParser->getGroup()->insertIntoGrid(ScopeGrid.get(), nullptr);
        pSceneParser->grid = ScopeGrid.get();
    }
}

void Reflect(Vec3f& in, Vec3f& norm, Vec3f& out)
{
    float project = in.Dot3(norm);
    out = in - 2 * project * norm;
    out.Normalize();
}

void Refract(Vec3f& in, Vec3f& norm, Vec3f& out, float eta)
{
    float NdotI = norm.Dot3(in);
    float k = 1 - eta * eta * (1.0 - NdotI * NdotI);
    if (k < 0)
        out = Vec3f();
    else
        out = in * eta - (eta * NdotI + sqrt(k)) * norm;
    out.Normalize();
}

Vec3f RayTracer::traceRay(Ray& ray, float tmin, int bounces, float weight,
		float indexOfRefraction, Hit& hit) const
{
    RayTracingStats::IncrementNumNonShadowRays();
    bool intersected = false;

    if (visualize_grid)
    {
        bool intersected = pSceneParser->grid->intersectVisualize(ray, hit, tmin);
        if (intersected)
        {
            Vec3f radiance(0, 0, 0);
            Vec3f albedo = hit.getMaterial()->getDiffuseColor();
            Vec3f::Mult(radiance, albedo, pSceneParser->getAmbientLight());
            Vec3f position = hit.getIntersectionPoint();

            for (int k = 0; k < pSceneParser->getNumLights(); k++)
            {
                Light* light = pSceneParser->getLight(k);

                bool inShadow = false;
                //if (mUseShadow)
                //{
                //    Vec3f hitPos = hit.getIntersectionPoint();
                //    Vec3f normal = hit.getNormal();
                //    hitPos += 0.001 * normal;
                //    inShadow = shadowRay(hitPos, light);
                //}

                Vec3f dir, col;
                light->getIllumination(position, dir, col);
                Vec3f outrad = hit.getMaterial()->Shade(ray, hit, dir, col);

                if (!inShadow)
                    radiance += outrad;
            }

            return radiance;
        }
        return pSceneParser->getBackgroundColor();
    }
    else if (pSceneParser->grid != nullptr)
    {
        intersected = pSceneParser->grid->intersect(ray, hit, tmin);
    }
    else
    {
        Group* group = pSceneParser->getGroup();
        intersected = group->intersect(ray, hit, tmin);
    }

    RayTree::SetMainSegment(ray, 0, hit.getT());

    if (intersected)
    {
        // Shading
        Vec3f radiance(0, 0, 0);
        Vec3f albedo = hit.getMaterial()->getDiffuseColor();
        Vec3f::Mult(radiance, albedo, pSceneParser->getAmbientLight());
        Vec3f position = hit.getIntersectionPoint();
        Vec3f normal = hit.getNormal();

        if (!mShadeback && normal.Dot3(ray.getDirection()) > 0)
        {
            return black;
        }
        //return hit.getMaterial()->getDiffuseColor();
        for (int k = 0; k < pSceneParser->getNumLights(); k++)
        {
            Light* light = pSceneParser->getLight(k);

            bool inShadow = false;
            if (mUseShadow)
            {
                Vec3f hitPos = hit.getIntersectionPoint();
                Vec3f normal = hit.getNormal();
                hitPos += 0.001 * normal;
                inShadow = shadowRay(hitPos, light);
            }

            Vec3f dir, col;
            light->getIllumination(position, dir, col);
            Vec3f outrad = hit.getMaterial()->Shade(ray, hit, dir, col);

            if (!inShadow)
                radiance += outrad;
        }

        Vec3f refectiveColor = hit.getMaterial()->getReflectiveColor();
        if (refectiveColor != Vec3f())
        {
            //Ray()
            Vec3f inRay = ray.getDirection();
            Vec3f nor = hit.getNormal();
            Vec3f outRay;
            Reflect(inRay, normal, outRay);
            Ray reflectRay(hit.getIntersectionPoint() + 0.001 * hit.getNormal(), outRay);
            radiance += refectiveColor * iteratorRay(reflectRay, 1, tmin, RayType::Reflect, refectiveColor);
        }

        Vec3f transparentColor = hit.getMaterial()->getTransparentColor();
        if (transparentColor != Vec3f())
        {
            //Ray()
            Vec3f inRay = ray.getDirection();
            Vec3f nor = hit.getNormal();
            Vec3f outRay;
            // Ray is getting out of the material
            if (nor.Dot3(ray.getDirection()) >= 0)
            {
                Refract(inRay, nor, outRay, hit.getMaterial()->getIndexOfRefraction());
            }
            else
            {
                Refract(inRay, nor, outRay, 1.0f / hit.getMaterial()->getIndexOfRefraction());
            }
            Ray refractRay(hit.getIntersectionPoint() - 0.001 * hit.getNormal(), outRay);
            radiance += transparentColor * iteratorRay(refractRay, 1, tmin, RayType::Refract, transparentColor);
        }

        return radiance;
    }

	return invalid;
}

bool RayTracer::shadowRay(Vec3f& position, Light* light) const
{
    RayTracingStats::IncrementNumShadowRays();

    Hit hit(999999, black_mat, Vec3f(0, 0, 0));
    float distance;
    Vec3f direction, color;
    light->getIllumination(position, direction, color, distance);
    Ray ray(position, direction);
    float tmin = 0.0001;
    bool intersected = false;
    if (pSceneParser->grid != nullptr)
    {
        intersected = pSceneParser->grid->intersect(ray, hit, tmin);
    }
    else
    {
        Group* group = pSceneParser->getGroup();
        intersected = group->intersect(ray, hit, tmin);
    }

    if (intersected && hit.getT() > distance)
    {
        intersected = false;
        RayTree::AddShadowSegment(ray, 0, hit.getT() + 0.1);
    }
    else
    {
        RayTree::AddShadowSegment(ray, 0, distance + 0.1);

    }
    //float distance = hit.getT();
    return intersected;
}

Vec3f RayTracer::iteratorRay(Ray& ray, int bounces, float tmin, RayType type, Vec3f weight) const
{
    RayTracingStats::IncrementNumNonShadowRays();

    Vec3f radiance(0, 0, 0);

    // If reach max iteration
    if (bounces > mMaxBounce)
        return radiance;

    // Else, calc the radiance
    Hit hit(999999, black_mat, Vec3f(0, 0, 0));
    bool intersected = false;
    if (pSceneParser->grid != nullptr)
    {
        intersected = pSceneParser->grid->intersect(ray, hit, tmin);
    }
    else
    {
        Group* group = pSceneParser->getGroup();
        intersected = group->intersect(ray, hit, tmin);
    }

    switch (type)
    {
    case RayTracer::RayType::Reflect:
        RayTree::AddReflectedSegment(ray, 0, hit.getT());
        break;
    case RayTracer::RayType::Refract:
        RayTree::AddTransmittedSegment(ray, 0, hit.getT());
        break;
    default:
        break;
    }
    if (intersected)
    {
        // Shading
        Vec3f albedo = hit.getMaterial()->getDiffuseColor();
        Vec3f::Mult(radiance, albedo, pSceneParser->getAmbientLight());
        Vec3f position = hit.getIntersectionPoint();
        Vec3f normal = hit.getNormal();
        Vec3f hitNormal = (normal.Dot3(ray.getDirection()) > 0) ? -1 * normal : normal;

        if (!mShadeback && normal.Dot3(ray.getDirection()) > 0)
        {
            return black;
        }
        
        //if (mShadeback && normal.Dot3(ray.getDirection()) < 0)
        {
            for (int k = 0; k < pSceneParser->getNumLights(); k++)
            {
                Light* light = pSceneParser->getLight(k);

                bool inShadow = false;
                if (mUseShadow)
                {
                    Vec3f hitPos = hit.getIntersectionPoint();
                    hitPos += 0.001 * normal;
                    inShadow = shadowRay(hitPos, light);
                }

                Vec3f dir, col;
                light->getIllumination(position, dir, col);
                Vec3f outrad = hit.getMaterial()->Shade(ray, hit, dir, col);

                if (!inShadow)
                    radiance += outrad;
            }

        }

        Vec3f refectiveColor = hit.getMaterial()->getReflectiveColor();
        if (refectiveColor != Vec3f())
        {
            //Ray()
            Vec3f inRay = ray.getDirection();
            Vec3f nor = hitNormal;
            Vec3f outRay;
            Reflect(inRay, normal, outRay);
            Ray reflectRay(hit.getIntersectionPoint() + 0.001 * hitNormal, outRay);
            Vec3f iterRes = refectiveColor * iteratorRay(reflectRay, bounces + 1, tmin, RayType::Reflect, weight * refectiveColor);
            if ((weight).Length() > mCutoffweight)
                radiance += iterRes;

        }

        Vec3f transparentColor = hit.getMaterial()->getTransparentColor();
        if (transparentColor != Vec3f())
        {
            //Ray()
            Vec3f inRay = ray.getDirection();
            Vec3f nor = hit.getNormal();
            Vec3f outRay;
            // Ray is getting out of the material
            if (nor.Dot3(ray.getDirection()) >= 0)
            {
                Refract(inRay, hitNormal, outRay, hit.getMaterial()->getIndexOfRefraction());
            }
            else
            {
                Refract(inRay, hitNormal, outRay, 1.0f / hit.getMaterial()->getIndexOfRefraction());
            }
            Ray refractRay(hit.getIntersectionPoint() - 0.001 * hitNormal, outRay);
            Vec3f iterRes = transparentColor * iteratorRay(refractRay, bounces + 1, tmin, RayType::Refract, weight * transparentColor);
            if ((weight).Length() > mCutoffweight)
                radiance += iterRes;
        }

        return radiance;
    }
    else
    {
        if (type == RayTracer::RayType::Reflect)
            radiance += pSceneParser->getBackgroundColor();

        return radiance;
    }

    return radiance;
}