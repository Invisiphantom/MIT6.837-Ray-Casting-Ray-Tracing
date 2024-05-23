#include "Material.h"

Vector3f Material::shade(const Ray& ray,                  // 视线
                         const Hit& hit,                  // 交点
                         const Vector3f& dirToLight,      // 交点到光源的方向
                         const Vector3f& lightIntensity)  // 光源颜色
{
    Vector3f N = hit.getNormal().normalized();  // 交点处法向量
    Vector3f L = dirToLight.normalized();       // 交点到光源的方向
    Vector3f I_diffuse =
        std::max(Vector3f::dot(N, L), 0.0f) * lightIntensity * _diffuseColor;  // 漫反射项

    Vector3f R = (2 * Vector3f::dot(L, N) * N - L).normalized();  // 理想反射矢量
    Vector3f I_spec = std::pow(std::max(Vector3f::dot(L, R), 0.0f), _shininess) *
                      lightIntensity * _specularColor;  // 镜面反射项

    return I_diffuse + I_spec;
}
