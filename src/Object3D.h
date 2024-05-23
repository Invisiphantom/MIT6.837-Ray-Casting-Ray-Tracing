#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "Ray.h"
#include "Material.h"

#include <string>

class Object3D {
   public:
    Object3D() { material = NULL; }
    virtual ~Object3D() {}
    Object3D(Material* material) { this->material = material; }
    std::string getType() const { return type; }
    Material* getMaterial() const { return material; }
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const = 0;

    std::string type;
    Material* material;  // 物体材质
};

class Sphere : public Object3D {
   public:
    Sphere() {
        _center = Vector3f(0.0, 0.0, 0.0);
        _radius = 1.0f;
    }
    Sphere(const Vector3f& center, float radius, Material* material)
        : Object3D(material), _center(center), _radius(radius) {}
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;

   private:
    Vector3f _center;  // 球心位置
    float _radius;     // 球体半径
};

class Group : public Object3D {
   public:
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;
    void addObject(Object3D* obj);
    int getGroupSize() const;

   private:
    std::vector<Object3D*> m_members;
};

class Plane : public Object3D {
   public:
    Plane(const Vector3f& normal, float d, Material* material)
        : Object3D(material), _normal(normal), _d(d) {}
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;

   private:
    Vector3f _normal;  // 平面法向量
    float _d;          // 原点距离
};

class Triangle : public Object3D {
   public:
    Triangle(const Vector3f& a,
             const Vector3f& b,
             const Vector3f& c,
             const Vector3f& na,
             const Vector3f& nb,
             const Vector3f& nc,
             Material* m)
        : Object3D(m), _v{a, b, c}, _normals{na, nb, nc} {}
    virtual bool intersect(const Ray& ray, float tmin, Hit& hit) const override;
    
    const Vector3f& getVertex(int index) const {
        assert(index < 3);
        return _v[index];
    }

    const Vector3f& getNormal(int index) const {
        assert(index < 3);
        return _normals[index];
    }

   private:
    Vector3f _v[3]; // 三顶点坐标
    Vector3f _normals[3]; // 三顶点法向量
};


// TODO implement this class
// So that the intersect function first transforms the ray
// Add more fields as necessary
class Transform : public Object3D {
   public:
    Transform(const Matrix4f& m, Object3D* obj)
    : Object3D(obj->getMaterial()), _object(obj), _mat(m) {
        _matInv = _mat.inverse();
        _matInvT = _matInv.transposed();
    }
    virtual bool intersect(const Ray& r, float tmin, Hit& h) const override;

   private:
    Object3D* _object;  // un-transformed object
    Matrix4f _mat, _matInv, _matInvT;  // transformation matrix
};

#endif
