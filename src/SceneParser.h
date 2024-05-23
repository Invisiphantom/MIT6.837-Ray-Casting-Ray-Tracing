#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <cassert>
#include <vector>
#include <vecmath.h>

#include "SceneParser.h"
#include "Camera.h"
#include "CubeMap.h"
#include "Light.h"
#include "Material.h"
#include "Object3D.h"
#include "Mesh.h"

#define MAX_PARSER_TOKEN_LENGTH 100

class SceneParser {
   public:
    SceneParser(const std::string& filename);
    ~SceneParser();

    Camera* getCamera() const { return _camera; }

    Vector3f getBackgroundColor(const Vector3f& dir) const {
        if (_cubemap) {
            return _cubemap->getTexel(dir);
        } else {
            return _background_color;
        }
    }

    const Vector3f& getAmbientLight() const { return _ambient_light; }

    int getNumLights() const { return _num_lights; }

    Light* getLight(int i) const {
        assert(i >= 0 && i < _num_lights);
        return lights[i];
    }

    int getNumMaterials() const { return _num_materials; }

    Material* getMaterial(int i) const {
        assert(i >= 0 && i < _num_materials);
        return _materials[i];
    }

    Group* getGroup() const { return _group; }

    std::vector<Light*> lights;  // 光源数组

   private:
    void parseFile();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light* parseDirectionalLight();
    Light* parsePointLight();
    void parseMaterials();
    Material* parseMaterial();

    Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    Sphere* parseSphere();
    Plane* parsePlane();
    Triangle* parseTriangle();
    Mesh* parseTriangleMesh();
    Transform* parseTransform();
    CubeMap* parseCubeMap();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);
    Vector3f readVector3f();
    Vector2f readVec2f();
    float readFloat();
    int readInt();

    std::string _basepath;              // 配置文件目录
    FILE* _file;                        // 配置文件
    Camera* _camera;                    // 相机配置
    Vector3f _background_color;         // 背景颜色
    Vector3f _ambient_light;            // 背景环境光
    int _num_lights;                    // 光源数量
    int _num_materials;                 // 材质数量
    std::vector<Material*> _materials;  // 材质数组
    std::vector<Object3D*> _objects;    // 物体数组
    Material* _current_material;        // 当前物体对应的材质
    Group* _group;                      // 物体组 vector<Object3D*> m_members
    CubeMap* _cubemap;                  // 背景盒子贴图
};

#endif  // SCENE_PARSER_H
