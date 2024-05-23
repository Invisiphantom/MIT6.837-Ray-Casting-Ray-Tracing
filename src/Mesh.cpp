#include "Mesh.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

#include <map>
#include <set>

struct Edge {
    int v1, v2;
    bool operator<(const Edge& e) const { return v1 < e.v1 || (v1 == e.v1 && v2 < e.v2); }
};

Vector3f computeNewVertex(const Vector3f& v1,
                          const Vector3f& v2,
                          const Vector3f& v3,
                          const Vector3f& v4) {
    return (3.0f / 8.0f) * (v1 + v2) + (1.0f / 8.0f) * (v3 + v4);
}

// LOOP细分函数
void subdivideMesh(std::vector<Vector3f>& vertices, std::vector<ObjTriangle>& triangles) {
    std::map<Edge, int> edgeVertexMap;
    std::vector<Vector3f> newVertices = vertices;
    std::vector<ObjTriangle> newTriangles;

    // Step 1: 计算新的顶点并插入到newVertices中
    for (auto& tri : triangles) {
        for (int i = 0; i < 3; i++) {
            int v1 = tri[i];
            int v2 = tri[(i + 1) % 3];
            if (v1 > v2)
                std::swap(v1, v2);
            Edge edge{v1, v2};
            if (edgeVertexMap.find(edge) == edgeVertexMap.end()) {
                int v3 = tri[(i + 2) % 3];
                Vector3f v4;
                bool found = false;
                for (auto& t : triangles) {
                    if (t != tri && (t[0] == v1 || t[1] == v1 || t[2] == v1) &&
                        (t[0] == v2 || t[1] == v2 || t[2] == v2)) {
                        for (int j = 0; j < 3; j++) {
                            if (t[j] != v1 && t[j] != v2) {
                                v4 = vertices[t[j]];
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found)
                        break;
                }
                if (!found)
                    v4 = vertices[v3];  // 边界情况
                Vector3f newVertex =
                    computeNewVertex(vertices[v1], vertices[v2], vertices[v3], v4);
                edgeVertexMap[edge] = newVertices.size();
                newVertices.push_back(newVertex);
            }
        }
    }

    // Step 2: 重新连接面片
    for (auto& tri : triangles) {
        int v0 = tri[0];
        int v1 = tri[1];
        int v2 = tri[2];
        int e0 = edgeVertexMap[{std::min(v0, v1), std::max(v0, v1)}];
        int e1 = edgeVertexMap[{std::min(v1, v2), std::max(v1, v2)}];
        int e2 = edgeVertexMap[{std::min(v2, v0), std::max(v2, v0)}];

        newTriangles.push_back({v0, e0, e2});
        newTriangles.push_back({v1, e1, e0});
        newTriangles.push_back({v2, e2, e1});
        newTriangles.push_back({e0, e1, e2});
    }

    // 更新顶点和三角形数据
    vertices = newVertices;
    triangles = newTriangles;
}

Mesh::Mesh(const std::string& filename, Material* material) : Object3D(material) {
    std::ifstream f;
    f.open(filename.c_str());
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }

    std::vector<Vector3f> v;         // 顶点数组
    std::vector<ObjTriangle> t;      // 三角形数组
    std::vector<Vector3f> n;         // 法向量数组
    std::vector<Vector2f> texCoord;  // 纹理坐标数组

    const std::string vTok("v");     // 顶点标记
    const std::string fTok("f");     // 面标记
    const std::string texTok("vt");  // 纹理标记
    const char bslash = '/', space = ' ';
    std::string tok;
    std::string line;
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {  // 添加顶点
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            v.push_back(vec);
        } else if (tok == fTok) {                          // 添加三角形
            if (line.find(bslash) != std::string::npos) {  // 如果有纹理
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);  // f 5/1 1/2 4/3
                ObjTriangle trig;
                facess >> tok;
                for (int ii = 0; ii < 3; ii++) {
                    facess >> trig[ii] >> trig.texID[ii];
                    trig[ii]--;
                    trig.texID[ii]--;
                }
                t.push_back(trig);
            } else {  // 没有纹理
                ObjTriangle trig;
                for (int ii = 0; ii < 3; ii++) {
                    ss >> trig[ii];
                    trig[ii]--;
                    trig.texID[ii] = 0;
                }
                t.push_back(trig);
            }
        } else if (tok == texTok) {  // 添加纹理坐标
            Vector2f texcoord;
            ss >> texcoord[0] >> texcoord[1];
            texCoord.push_back(texcoord);
        }
    }
    f.close();

    // LOOP曲面细分器
    // subdivideMesh(v, t);

    // 将新数据写入文件
    // std::ofstream out("../subdivided.obj");
    // for (int i = 0; i < v.size(); i++) {
    //     out << "v " << v[i][0] << " " << v[i][1] << " " << v[i][2] << "\n";
    // }
    // for (int i = 0; i < t.size(); i++) {
    //     out << "f " << t[i][0] + 1 << " " << t[i][1] + 1 << " " << t[i][2] + 1 << "\n";
    // }

    // Compute normals
    // will smooth normals.
    // if sharp edges required, build OBJ with no shared vertices.
    n.resize(v.size());  // 计算法向量
    for (int ii = 0; ii < t.size(); ii++) {
        Vector3f a = v[t[ii][1]] - v[t[ii][0]];
        Vector3f b = v[t[ii][2]] - v[t[ii][0]];
        Vector3f normal = Vector3f::cross(a, b).normalized();
        for (int jj = 0; jj < 3; jj++) {
            n[t[ii][jj]] += normal;  // 累加各三角形的法向量
        }
    }
    for (int ii = 0; ii < v.size(); ii++) {
        n[ii] = n[ii] / n[ii].abs();  // 归一化
    }

    // Set up triangles
    for (int i = 0; i < t.size(); i++) {
        Triangle triangle(v[t[i][0]], v[t[i][1]], v[t[i][2]], n[t[i][0]], n[t[i][1]],
                          n[t[i][2]], getMaterial());
        _triangles.push_back(triangle);
    }

    octree.build(this);
}

bool Mesh::intersect(const Ray& r, float tmin, Hit& h) const {
#if 1
    ray = &r;
    hit = &h;
    tm = tmin;
    return octree.intersect(r);
#else
    bool result = false;
    for (Triangle t : _triangles) {
        if (t.intersect(r, tmin, h)) {
            result = true;
        }
    }
    return result;
#endif
}

bool Mesh::intersectTrig(int idx, const Ray& r) const {
    const Triangle& triangle = _triangles[idx];
    bool result = triangle.intersect(r, tm, *hit);
    return result;
}
