#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"

#include <vecmath.h>
#include <float.h>
#include <cmath>

class Camera
{
public:
    virtual ~Camera() {}

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual float getTMin() const = 0;
};

/// Fill in functions and add more fields if necessary
class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(const Vector3f &center,
        const Vector3f &direction,
        const Vector3f &up,
        float angleradians) :
        _center(center),
        _direction(direction.normalized()),
        _up(up),
        _angle(angleradians)
    {
        _horizontal = Vector3f::cross(direction, up).normalized();
    }

    virtual Ray generateRay(const Vector2f &point) override
    {
        // 输入坐标 x[-1,1] y[-1,1]
        // BEGIN STARTER
        float d = 1.0f / (float)std::tan(_angle / 2.0f);
        Vector3f newDir = d * _direction + point[0] * _horizontal + point[1] * _up;
        newDir = newDir.normalized(); // 当前图片像素对应的光线向量

        return Ray(_center, newDir);
        // END STARTER
    }

    virtual float getTMin() const override
    {
        return 0.0001f;
    }

private:
    Vector3f _center; // 相机位置
    Vector3f _direction; // 相机方向
    Vector3f _up; // 相机上方向
    float _angle; // 相机视角弧度
    Vector3f _horizontal; // 相机右方向
};

#endif //CAMERA_H
