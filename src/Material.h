#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "Ray.h"
#include "Image.h"
#include "Vector3f.h"

#include <string>

class Material
{
  public:
    Material(const Vector3f &diffuseColor, 
             const Vector3f &specularColor = Vector3f::ZERO, 
             float shininess = 0.0f) :
        _diffuseColor(diffuseColor),
        _specularColor(specularColor),
        _shininess(shininess)
    { }

    const Vector3f & getDiffuseColor() const {
        return _diffuseColor;
    }

    const Vector3f & getSpecularColor() const {
        return _specularColor;
    }

    const float getShininess() const {
        return _shininess;
    }

    Vector3f shade(const Ray &ray,
        const Hit &hit,
        const Vector3f &dirToLight,
        const Vector3f &lightIntensity);

protected:

    Vector3f _diffuseColor; // 漫反射颜色
    Vector3f _specularColor; // 镜面反射颜色
    float   _shininess; // 光泽度
};

#endif // MATERIAL_H
