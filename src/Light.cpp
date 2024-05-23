#include "Light.h"
void DirectionalLight::getIllumination(const Vector3f& p,
                                       Vector3f& tolight,
                                       Vector3f& intensity,
                                       float& distToLight) const {
    // the direction to the light is the opposite of the
    // direction of the directional light source

    // BEGIN STARTER
    tolight = -_direction;
    intensity = _color;
    distToLight = std::numeric_limits<float>::max();
    // END STARTER
}

void PointLight::getIllumination(const Vector3f& p,    // 交点位置
                                 Vector3f& tolight,    // 交点到光源的方向
                                 Vector3f& intensity,  // 光源强度
                                 float& distToLight    // 交点到光源的距离
) const {
    tolight = (_position - p).normalized();
    distToLight = tolight.abs();
    intensity = _color / (_falloff * distToLight * distToLight);
}
