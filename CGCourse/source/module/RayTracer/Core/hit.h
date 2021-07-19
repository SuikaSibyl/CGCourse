#ifndef _HIT_H
#define _HIT_H

#include "vectors.h"
#include "ray.h"
#include "../VersionControl.h"

class Material;

// ====================================================================
// ====================================================================

#if (RTVersion==2)
class Hit_v2 {

public:

    // CONSTRUCTOR & DESTRUCTOR
    Hit_v2() { material = NULL; }
    Hit_v2(float _t, Material* m, Vec3f n) {
        t = _t; material = m; normal = n;
    }
    Hit_v2(const Hit_v2& h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
        intersectionPoint = h.intersectionPoint;
    }
    ~Hit_v2() {}

    // ACCESSORS
    float getT() const { return t; }
    Material* getMaterial() const { return material; }
    Vec3f getNormal() const { return normal; }
    Vec3f getIntersectionPoint() const { return intersectionPoint; }

    // MODIFIER
    void set(float _t, Material* m, Vec3f n, const Ray& ray) {
        t = _t; material = m; normal = n;
        intersectionPoint = ray.pointAtParameter(t);
    }

private:

    // REPRESENTATION
    float t;
    Material* material;
    Vec3f normal;
    Vec3f intersectionPoint;

};

inline ostream& operator<<(ostream& os, const Hit_v2& h) {
    os << "Hit_v2 <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}
// ====================================================================
// ====================================================================
#endif

// If in assignment 1
#if (RTVersion==1)
// ====================================================================
// ====================================================================

class Hit_v1 {

public:

    // CONSTRUCTOR & DESTRUCTOR
    Hit_v1() { material = NULL; }
    Hit_v1(float _t, Material* m) {
        t = _t; material = m;
    }
    Hit_v1(const Hit_v1& h) {
        t = h.t;
        material = h.material;
        intersectionPoint = h.intersectionPoint;
    }
    ~Hit_v1() {}

    // ACCESSORS
    float getT() const { return t; }
    Material* getMaterial() const { return material; }
    Vec3f getIntersectionPoint() const { return intersectionPoint; }

    // MODIFIER
    void set(float _t, Material* m, const Ray& ray) {
        t = _t; material = m;
        intersectionPoint = ray.pointAtParameter(t);
    }

private:

    // REPRESENTATION
    float t;
    Material* material;
    Vec3f intersectionPoint;

};

inline ostream& operator<<(ostream& os, const Hit_v1& h) {
    os << "Hit_v1 <t:" << h.getT() << ">";
    return os;
}
// ====================================================================
// ====================================================================

#endif // (RTVersion==0)
#endif