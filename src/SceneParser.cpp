#include <cstdio>
#include <cstring>
#include <cstdlib>
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "SceneParser.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

#include "Object3D.h"

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

static void _PostError(const std::string& msg) {
    std::cout << msg;
    exit(1);
}

SceneParser::SceneParser(const std::string& filename)
    : _file(NULL),
      _camera(NULL),
      _background_color(0.5, 0.5, 0.5),  // 背景颜色
      _ambient_light(0, 0, 0),           // 环境光
      _num_lights(0),
      _num_materials(0),
      _current_material(NULL),
      _group(NULL),
      _cubemap(NULL) {
    // parse the file
    assert(!filename.empty());

    if (filename.size() <= 4) {
        _PostError("ERROR: Wrong file name extension\n");
    }

    size_t last_sep = filename.find_last_of("\\/");  // 查找路径分隔符
    if (last_sep == std::string::npos) {  // 没有找到路径分隔符
        _basepath = "";
    } else {  // 找到路径分隔符
        _basepath = filename.substr(0, last_sep + 1);
    }

    std::string ext = filename.substr(filename.size() - 4, 4);
    if (ext != ".txt") {  // 如果文件名后缀不是.txt
        _PostError("ERROR: Wrong file name extension\n");
    }

    _file = fopen(filename.c_str(), "r");

    // FIXME extract base path from scene file path
    if (_file == NULL) {
        _PostError(std::string("Cannot open scene file ") + filename + "\n");
    }

    parseFile();    // 解析配置文件
    fclose(_file);  // 关闭文件
    _file = NULL;

    // if no lights are specified, set ambient light to white
    // (do solid color ray casting)
    if (_num_lights == 0) {
        std::cerr << "WARNING: No lights specified\n";
        _ambient_light = Vector3f(1, 1, 1);
    }
}

SceneParser::~SceneParser() {
    // FIXME Object3Ds leak. must keep track and delete.
    delete _group;
    delete _camera;
    for (auto* material : _materials) {
        delete material;
    }
    for (auto* light : lights) {
        delete light;
    }
    for (auto* object : _objects) {
        delete object;
    }
    delete _cubemap;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera,
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera"))
            parsePerspectiveCamera();  // 初始化相机配置
        else if (!strcmp(token, "Background"))
            parseBackground();  // 初始化背景配置
        else if (!strcmp(token, "Lights"))
            parseLights();  // 初始化光源配置
        else if (!strcmp(token, "Materials"))
            parseMaterials();  // 初始化材质配置
        else if (!strcmp(token, "Group"))
            _group = parseGroup();  // 初始化物体组配置
        else {
            _PostError(std::string("Unknown token in parseFile: '") + token +
                       "'\n");
            exit(1);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();  // 相机中心位置
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();  // 相机观察方向
    getToken(token);
    assert(!strcmp(token, "up"));
    Vector3f up = readVector3f();  // 相机向上方向
    getToken(token);
    assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();  // 相机视角
    float angle_radians = (float)DegreesToRadians(angle_degrees);  // 角度转弧度
    getToken(token);
    assert(!strcmp(token, "}"));
    _camera = new PerspectiveCamera(center, direction, up, angle_radians);
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            _background_color = readVector3f();  // 背景颜色
        } else if (!strcmp(token, "ambientLight")) {
            _ambient_light = readVector3f();  // 环境光
        } else if (!strcmp(token, "cubeMap")) {
            _cubemap = parseCubeMap();  // 背景盒子贴图
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

CubeMap* SceneParser::parseCubeMap() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    return new CubeMap(_basepath + token);
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "numLights"));
    _num_lights = readInt();  // 光源数量
    int count = 0;
    while (_num_lights > count) {
        getToken(token);
        if (!strcmp(token, "DirectionalLight")) {
            lights.push_back(parseDirectionalLight());  // 添加方向光
        } else if (strcmp(token, "PointLight") == 0) {
            lights.push_back(parsePointLight());  // 添加点光源
        } else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}

Light* SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();  // 方向光方向
    getToken(token);
    assert(!strcmp(token, "color"));
    Vector3f color = readVector3f();  // 方向光颜色
    getToken(token);
    assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light* SceneParser::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vector3f position, color;
    float falloff = 0;
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "position")) {
            position = readVector3f();  // 点光源位置
        } else if (!strcmp(token, "color")) {
            color = readVector3f();  // 点光源颜色
        } else if (!strcmp(token, "falloff")) {
            falloff = readFloat();  // 点光源衰减系数
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new PointLight(position, color, falloff);
}

// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "numMaterials"));
    _num_materials = readInt();  // 材质数量
    int count = 0;
    while (_num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") || !strcmp(token, "PhongMaterial")) {
            _materials.push_back(parseMaterial());  // 添加材质
        } else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}

Material* SceneParser::parseMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    Vector3f diffuseColor(1, 1, 1);   // 漫反射颜色
    Vector3f specularColor(0, 0, 0);  // 镜面反射颜色
    float shininess = 0;              // 光泽度
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();  // 漫反射颜色
        } else if (strcmp(token, "specularColor") == 0) {
            specularColor = readVector3f();  // 镜面反射颜色
        } else if (strcmp(token, "shininess") == 0) {
            shininess = readFloat();  // 光泽度
        } else if (strcmp(token, "bump") == 0) {
            getToken(token);
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material* answer = new Material(diffuseColor, specularColor, shininess);

    return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = NULL;
    if (!strcmp(token, "Group"))
        answer = (Object3D*)parseGroup();  // 解析物体组
    else if (!strcmp(token, "Sphere"))
        answer = (Object3D*)parseSphere();  // 解析球体
    else if (!strcmp(token, "Plane"))
        answer = (Object3D*)parsePlane();  // 解析平面
    else if (!strcmp(token, "Triangle"))
        answer = (Object3D*)parseTriangle();  // 解析三角形
    else if (!strcmp(token, "TriangleMesh"))
        answer = (Object3D*)parseTriangleMesh();  // 解析三角网格
    else if (!strcmp(token, "Transform"))
        answer = (Object3D*)parseTransform();  // 解析变换
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();  // 物体数量

    Group* answer = new Group();

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();  // 获取对应的材质索引
            assert(index >= 0 && index <= getNumMaterials());
            _current_material = getMaterial(index);
        } else {
            Object3D* object = parseObject(token);
            assert(object != NULL);
            answer->addObject(object);  // 添加物体
            count++;
            _objects.push_back(object);  // 添加物体
        }
    }
    getToken(token);
    assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();  // 球心位置
    getToken(token);
    assert(!strcmp(token, "radius"));
    float radius = readFloat();  // 球半径
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(_current_material != NULL);
    return new Sphere(center, radius, _current_material);
}

Plane* SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(_current_material != NULL);
    return new Plane(normal, offset, _current_material);
}

Triangle* SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(_current_material != NULL);
    Vector3f a = v1 - v0;
    Vector3f b = v2 - v0;
    Vector3f n = Vector3f::cross(a, b).normalized();
    return new Triangle(v0, v1, v2, n, n, n, _current_material);
}

Mesh* SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token);
    assert(!strcmp(token, "}"));
    const char* ext = &filename[strlen(filename) - 4];  // 获取物体文件后缀
    assert(!strcmp(ext, ".obj"));
    Mesh* answer = new Mesh(_basepath + filename, _current_material);

    return answer;
}

Transform* SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D* object = NULL;
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in transformations:
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        } else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        } else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        } else if (!strcmp(token, "XRotate")) {
            matrix = matrix *
                     Matrix4f::rotateX((float)DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "YRotate")) {
            matrix = matrix *
                     Matrix4f::rotateY((float)DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "ZRotate")) {
            matrix = matrix *
                     Matrix4f::rotateZ((float)DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert(!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = (float)DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert(!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }

    assert(object != NULL);
    getToken(token);
    assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(_file != NULL);  // 确保文件指针_file不为空
    int success = fscanf(
        _file, "%s ",
        token);  // 从文件中读取一个以空白字符分隔的标记，并存储到token数组中
    if (success == EOF) {  // 如果到达文件末尾
        token[0] = '\0';   // 将token设置为空字符串
        return 0;          // 返回0，表示没有读取到标记
    }
    return 1;  // 返回1，表示成功读取到标记
}

Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(_file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert(0);
    }
    return Vector3f(x, y, z);
}

Vector2f SceneParser::readVec2f() {
    float u, v;
    int count = fscanf(_file, "%f %f", &u, &v);
    if (count != 2) {
        printf("Error trying to read 2 floats to make a Vec2f\n");
        assert(0);
    }
    return Vector2f(u, v);
}

float SceneParser::readFloat() {
    float answer;
    int count = fscanf(_file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}

int SceneParser::readInt() {
    int answer;
    int count = fscanf(_file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}
