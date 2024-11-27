

#ifndef AS3_H
#define AS3_H
#include "Ray.h"
#include "Hittable.h"
bool triangle_hit(const Ray &ray, HitRecord *hit_record, Vec a_, Vec b_, Vec c_, Vec n_a_, Vec n_b_, Vec n_c_);
bool sphere_hit(const Ray &ray, HitRecord *hit_record, Vec o_, float r_, Material material_);
Vec SampleHemisphere_as3(const Vec& normal);
#endif //AS3_H
