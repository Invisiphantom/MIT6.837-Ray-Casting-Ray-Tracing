#include "Object3D.h"

// 判断球体是否与光线相交
bool Sphere::intersect(const Ray& r, float tmin, Hit& h) const {
    // BEGIN STARTER

    // We provide sphere intersection code for you.
    // You should model other intersection implementations after this one.

    // Locate intersection point ( 2 pts )
    const Vector3f& rayOrigin = r.getOrigin();  // 光线对应的相机位置
    const Vector3f& dir = r.getDirection();     // 光线方向

    Vector3f origin = rayOrigin - _center;  // 球心到光线起始点的方向向量

    // 球体和直线的交点方程
    float a = dir.absSquared();
    float b = 2 * Vector3f::dot(dir, origin);
    float c = origin.absSquared() - _radius * _radius;

    // no intersection
    if (b * b - 4 * a * c < 0)  // Delta<0 无解
        return false;

    float d = sqrt(b * b - 4 * a * c);  // sqrt(Delta)

    float tplus = (-b + d) / (2.0f * a);   // 交点1
    float tminus = (-b - d) / (2.0f * a);  // 交点2

    // 两交点都在相机后面, 光线无交
    if ((tplus < tmin) && (tminus < tmin))
        return false;

    float t = 10000;

    // 两交点都在相机前面
    if (tminus > tmin)
        t = tminus;

    // 一个交点在相机前面，一个在相机后面
    if ((tplus > tmin) && (tminus < tmin))
        t = tplus;

    if (t < h.getT())  // 如果找到一个更近的交点，更新命中信息
    {
        Vector3f normal = r.pointAtParameter(t) - _center;  // 交点处的法向量
        normal = normal.normalized();
        h.set(t, this->material, normal);  // 更新命中信息
        return true;
    }
    // END STARTER
    return false;
}

// Add object to group
void Group::addObject(Object3D* obj) {
    m_members.push_back(obj);
}

// Return number of objects in group
int Group::getGroupSize() const {
    return (int)m_members.size();
}

bool Group::intersect(const Ray& r, float tmin, Hit& h) const {
    bool hit = false;
    for (Object3D* o : m_members)      // 遍历所有物体
        if (o->intersect(r, tmin, h))  // 如果发生相交
            hit = true;
    return hit;
}

bool Plane::intersect(const Ray& r, float tmin, Hit& h) const {
    if (Vector3f::dot(_normal, r.getDirection()) == 0)
        return false;
    float t = (_d - Vector3f::dot(r.getOrigin(), _normal)) /
              Vector3f::dot(r.getDirection(), _normal);
    if (t < tmin || t > h.getT())
        return false;
    h.set(t, material, _normal);
    return true;
}

bool Triangle::intersect(const Ray& r, float tmin, Hit& h) const {
    Vector3f S = r.getOrigin() - _v[0];
    Vector3f E1 = _v[1] - _v[0];
    Vector3f E2 = _v[2] - _v[0];
    Vector3f S1 = Vector3f::cross(r.getDirection(), E2);
    Vector3f S2 = Vector3f::cross(S, E1);

    float S1E1 = Vector3f::dot(S1, E1);
    float t = Vector3f::dot(S2, E2) / S1E1;
    float b1 = Vector3f::dot(S1, S) / S1E1;
    float b2 = Vector3f::dot(S2, r.getDirection()) / S1E1;

    if (t > tmin && t < h.getT() && b1 > 0 && b2 > 0 && (1 - b1 - b2) > 0) {
        h.set(t, material,
              (_normals[0] + _normals[1] + _normals[2]).normalized());
        return true;
    }
    return false;
}

bool Transform::intersect(const Ray& r, float tmin, Hit& h) const {
    Vector3f orig_obj =
        (_matInv * Vector4f(r.getOrigin(), 1.0f)).homogenized().xyz();
    Vector3f dirc_obj = (_matInv * Vector4f(r.getDirection(), 0.0f)).xyz();
    float scale = dirc_obj.abs();
    Ray r_obj = Ray(orig_obj, dirc_obj.normalized());

    Hit h_obj = h;
    h_obj.t *= scale;
    if (!_object->intersect(r_obj, tmin, h_obj))
        return false;  // 在局部对象坐标系判断是否相交

    float t = h_obj.getT() / scale;
    Vector3f N = h_obj.getNormal();
    N = (_matInvT * Vector4f(N, 0.0f)).xyz().normalized();
    h.set(t, h_obj.getMaterial(), N);
    return true;
}