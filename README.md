## Project 2.1 Phong光照模型 - 代码实现

### PointLight::getIllumination()

(a)tolight：从场景中一个点指向光源的方向矢量(归一化后的)  
(b)intensity：此时的照明强度(RGB)  
(c)distToLight：场景点与光源之间的距离  

光强公式: $L(x_{\text{surf}})=\frac{I}{\alpha d^2}$  

```cpp
void PointLight::getIllumination(const Vector3f& p,    // 交点位置
                                 Vector3f& tolight,    // 交点到光源的方向
                                 Vector3f& intensity,  // 光源强度
                                 float& distToLight    // 交点到光源的距离
) const {
    tolight = (_position - p).normalized();
    distToLight = tolight.abs();
    intensity = _color / (_falloff * distToLight * distToLight);
}
```

### Material::shade()

漫反射强度: $I_{\text{diffuse}}=\text{clamp}(L\cdot N)\cdot L\cdot k_{\text{diffuse}}$  
镜面反射强度: $I_{\text{specular}}=\text{clamp}(R\cdot V)^{s}\cdot L\cdot k_{\text{specular}}$  
```cpp
Vector3f Material::shade(const Ray& ray,                  // 视线
                         const Hit& hit,                  // 交点
                         const Vector3f& dirToLight,      // 交点到光源的方向
                         const Vector3f& lightIntensity)  // 光源颜色
{
    Vector3f N = hit.getNormal().normalized();  // 交点处法向量
    Vector3f L = dirToLight.normalized();       // 交点到光源的方向
    Vector3f I_diffuse = std::max(Vector3f::dot(N, L), 0.0f) * 
                         lightIntensity * _diffuseColor;  // 漫反射项

    Vector3f R = (2 * Vector3f::dot(L, N) * N - L).normalized();  // 理想反射矢量
    Vector3f I_spec = std::pow(std::max(Vector3f::dot(L, R), 0.0f), _shininess) *
                      lightIntensity * _specularColor;  // 镜面反射项

    return I_diffuse + I_spec;
}
```





## Project 2.2 光线投射 - 代码实现


### Plane::intersect() - 判断平面相交

当光线r(t)与平面相交，设交点为$p=o+t*d$  
设p'为平面内一个点，则满足$(p–p')\cdot N=0$  
计算公式: $t=\frac{(p'-o)\cdot N}{d\cdot N}$  
```cpp
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
```

### Triangle::intersect() - 判断三角形相交

参照项目文档中的链接，使用Möller-Trumbore算法判断三角形与光线相交  
$\overrightarrow{P_0p}=b_1\cdot\overrightarrow{P_0P_1}+b_2\cdot\overrightarrow{P_0P_2}$  
$\implies o+t\overrightarrow{d}=(1-b_1-b_2)P_0+b_1P_1+b_2P_2$  
$\implies o-P_0=(P_1-P_0)b_1+(P_2-P_0)b_2-t\overrightarrow{d}$  
令$S=o-P_0,E_1=P_1-P_0,E_2=P_2-P_0$  
则得到$S=E_1b_1+E_2b_2-t\overrightarrow{d}$  
换成矩阵乘法: $\begin{bmatrix}-d&E_1&E_2\end{bmatrix}\begin{bmatrix}t\\b_1\\b_2\end{bmatrix}=S$  

根据Cramer法则解得:
1. $t=\frac{\det\begin{bmatrix}S&E_1&E_2\end{bmatrix}}{\det\begin{bmatrix}-d&E_1&E_2\end{bmatrix}}
=\frac{(S\times E_1)\cdot E_2}{(d\times E_2)\cdot E_1}=\frac{S_2\cdot E_2}{S_1\cdot E_1}$
2. $b_1=\frac{\det\begin{bmatrix}-d&S&E_2\end{bmatrix}}{\det\begin{bmatrix}-d&E_1&E_2\end{bmatrix}}
=\frac{(d\times E_2)\cdot S}{(d\times E_2)\cdot E_1}=\frac{S_1\cdot S}{S_1\cdot E_1}$
3. $b_2=\frac{\det\begin{bmatrix}-d&E_1&S\end{bmatrix}}{\det\begin{bmatrix}-d&E_1&E_2\end{bmatrix}}
=\frac{(S\times E_1)\cdot d}{(d\times E_2)\cdot E_1}=\frac{S_2\cdot d}{S_1\cdot E_1}$

```cpp
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
```

### Transform::intersect() - 判断变换后的物体相交

1. 将光线从世界坐标变换到局部对象坐标
2. 在局部对象坐标系中判断是否相交
3. 将交点的法向量变换回世界坐标系

```cpp
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
```




## Project 2.3 光线追踪与阴影投射

### Renderer::traceRay()

光线追踪: $I_{\text{total}}=I_{\text{direct}}+k_{\text{specular}}\cdot I_{\text{indirect}}$  
阴影投射: 如果交点到光源的路径上存在遮挡，则不计算该光源对交点的贡献  
```cpp
Vector3f Renderer::traceRay(const Ray& r,  // 当前图片像素对应的光线
                            float tmin,    // 0.001f 偏移距离
                            int bounces,   // 光追最大递归深度
                            Hit& h         // 光线的当前交点
) const {
    if (_scene.getGroup()->intersect(r, tmin, h))  // 如果与物体有相交
    {
        // 场景环境光
        Vector3f color = _scene.getAmbientLight() * h.getMaterial()->getDiffuseColor();
        Vector3f p = r.getOrigin() + r.getDirection() * h.getT();

        // 累加各光源对物体表面的光照
        for (int i = 0; i < _scene.getNumLights(); i++) {
            Vector3f tolight;                 // 交点到光源的方向
            Vector3f lightColor;              // 光源发出的颜色
            float disToLight;                 // 交点到光源的距离
            auto light = _scene.getLight(i);  // 获取当前光源
            light->getIllumination(p, tolight, lightColor, disToLight);

            // 测试阴影遮挡
            if (_args.shadows) {
                Hit h_test;
                Ray r_test = {p + tolight * 0.001f, tolight.normalized()};
                if (_scene.getGroup()->intersect(r_test, 0, h_test) &&
                    h_test.getT() < disToLight)
                    continue;  // 在交点到光源的路径上存在遮挡
            }
            color += h.getMaterial()->shade(r, h, tolight, lightColor);
        }

        // 递归光线追踪
        if (bounces > 0) {
            Vector3f N = h.getNormal().normalized();      // 交点处法向量
            Vector3f L = -r.getDirection().normalized();  // 交点到视点的方向
            Vector3f R = (2 * Vector3f::dot(L, N) * N - L).normalized();  // 理想反射矢量

            Hit h_ref;
            Ray r_ref = {p + R * 0.001f, R};
            color += h.getMaterial()->getSpecularColor() *
                     traceRay(r_ref, tmin, bounces - 1, h_ref);
        }
        return color;
    } else
        return _scene.getBackgroundColor(r.getDirection());  // 返回背景颜色
}
```


## Project 2 拓展 - 抗锯齿

### 抖动采样 -jitter

每次都对ndcx, ndcy加上一定的偏差  
偏差的值从0到1的像素宽度随机选取  
```cpp
std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
for (int i = 0; i < num_samples; i++) {
    float ndcy = 2 * ((y + dis(gen)) / (h - 1.0f)) - 1.0f;  // 抖动
    float ndcx = 2 * ((x + dis(gen)) / (w - 1.0f)) - 1.0f;  // 抖动
    ......
}
```

### 高斯滤波 -filter

先进行倍数是k的上采样(在高分辨率h×k,w×k下渲染，k=3)  
然后对生成的Image类型图像再下采样(得到低分辨率(h,w)图像)  

```cpp
    int w = _args.width;
    int h = _args.height;

    // 高斯滤波
    if (_args.filter == true) {
        const int k = 3;
        w *= k;
        h *= k;
    }

......

float ker[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
for (int y = 0; y < _args.height; y++) {
    for (int x = 0; x < _args.width; x++) {
        Vector3f color = {0, 0, 0};
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {
                int x_pos = x * 3 + i, y_pos = y * 3 + j;
                // 如果超出边界, 则跳过此次计算
                if (x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)
                    continue;
                color += image.getPixel(x_pos, y_pos) * ker[i + 1][j + 1];
            }
        output.setPixel(x, y, color / 16.0f);
    }
}
```

### Renderer::Render() - 整体代码

```cpp
void Renderer::Render() {
    int w = _args.width;
    int h = _args.height;

    // 高斯滤波
    if (_args.filter == true) {
        const int k = 3;
        w *= k;
        h *= k;
    }

    Image image(w, h);
    Image nimage(w, h);
    Image dimage(w, h);

    // 随机数生成器[-1,1]
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    Camera* cam = _scene.getCamera();  // 获取相机配置
    for (int y = 0; y < h; y++)        // 遍历图片的高度像素 [0,h)
    {
        std::cerr << "Rendering row " << y << " of " << h << std::endl;
        for (int x = 0; x < w; x++) {
            Hit hit;         // 当前光线的交点属性
            Vector3f color;  // 当前像素的颜色
            if (_args.jitter == false) {
                float ndcy = 2 * (y / (h - 1.0f)) - 1.0f;        // 标准化y坐标 [-1,1]
                float ndcx = 2 * (x / (w - 1.0f)) - 1.0f;        // 标准化x坐标 [-1,1]
                Ray r = cam->generateRay(Vector2f(ndcx, ndcy));  // 当前图片像素对应的光线
                color = traceRay(r, cam->getTMin(), _args.bounces, hit);
            } else {
                // 抖动采样
                int num_samples = 16;
                for (int i = 0; i < num_samples; i++) {
                    hit = {};
                    float ndcy = 2 * ((y + dis(gen)) / (h - 1.0f)) - 1.0f;  // 抖动
                    float ndcx = 2 * ((x + dis(gen)) / (w - 1.0f)) - 1.0f;  // 抖动
                    Ray r = cam->generateRay(Vector2f(ndcx, ndcy));
                    color += traceRay(r, cam->getTMin(), _args.bounces, hit);
                }
                color = color / num_samples;
            }

            image.setPixel(x, y, color);
            nimage.setPixel(x, y, (hit.getNormal() + 1.0f) / 2.0f);
            float range = (_args.depth_max - _args.depth_min);
            if (range)
                dimage.setPixel(x, y, Vector3f((hit.t - _args.depth_min) / range));
        }
    }

    if (_args.output_file.size()) {
        if (_args.filter == false)
            image.savePNG(_args.output_file);
        else {
            // 高斯滤波
            Image output(_args.width, _args.height);
            float ker[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
            for (int y = 0; y < _args.height; y++) {
                for (int x = 0; x < _args.width; x++) {
                    Vector3f color = {0, 0, 0};
                    for (int i = -1; i <= 1; i++)
                        for (int j = -1; j <= 1; j++) {
                            int x_pos = x * 3 + i, y_pos = y * 3 + j;
                            // 如果超出边界, 则跳过此次计算
                            if (x_pos < 0 || x_pos >= w || y_pos < 0 || y_pos >= h)
                                continue;
                            color += image.getPixel(x_pos, y_pos) * ker[i + 1][j + 1];
                        }
                    output.setPixel(x, y, color / 16.0f);
                }
            }
            output.savePNG(_args.output_file);
        }
    }

    if (_args.depth_file.size())
        dimage.savePNG(_args.depth_file);
    if (_args.normals_file.size())
        nimage.savePNG(_args.normals_file);
}
```




## 效果展示

### Phong光照模型

#### scene01_plane.txt - 球体与平面

```sh
./build/a2 
-input data/scene01_plane.txt     
-output output/01.png 
-size 1080 1080
```
![01.png](https://s2.loli.net/2024/05/23/hXPVs5U94BoZjND.png)

#### scene02_cube.txt - 立方体

```sh
./build/a2 -input data/scene02_cube.txt      
-output output/02.png 
-size 1080 1080 
```
![02.png](https://s2.loli.net/2024/05/23/m5ezvxjKwHNhFuY.png)

#### scene03_sphere.txt - 变换后的椭球体

```sh
./build/a2 -input data/scene03_sphere.txt    
-output output/03.png 
-size 1080 1080
```
![03.png](https://s2.loli.net/2024/05/23/IaDey1Zi4OvXAVG.png)

#### scene04_axes.txt - 坐标轴

```sh
./build/a2 -input data/scene04_axes.txt      
-output output/04.png 
-size 1080 1080
```
![04.png](https://s2.loli.net/2024/05/23/5rOBquH6mWeizhS.png)


#### scene05_bunny_200.txt - 兔子模型

```sh
./build/a2 -input data/scene05_bunny_200.txt 
-output output/05.png 
-size 1080 1080
```

![05.png](https://s2.loli.net/2024/05/23/Q9S3LqzI7jTYtrk.png)


----

<br>

### 光线追踪与阴影投射

```sh
./build/a2 -input data/scene07_arch.txt      
-output output/07.png 
-size 1080 1080 
-shadows 
-bounces 100
```
![07.png](https://s2.loli.net/2024/05/23/d6SL1tbJMNierBQ.png)




### 抖动采样与高斯滤波

#### 抖动采样 - jitter

```sh
./build/a2 -input data/scene07_arch.txt      
-output output/07_jitter.png        
-size 1080 1080 
-shadows 
-bounces 100 
-jitter
```

![07_filter.png](https://s2.loli.net/2024/05/23/kvYFX9csUG8lSzT.png)

#### 高斯滤波 - filter

```sh
./build/a2 
-input data/scene07_arch.txt      
-output output/07_filter.png        
-size 1080 1080 -shadows 
-bounces 100 
-filter
```

![07_jitter.png](https://s2.loli.net/2024/05/23/oWCfSOYeuXHMUzP.png)

#### 抗锯齿 - jitter & filter

```sh
./build/a2 -input data/scene07_arch.txt      
-output output/07-jitter_filter.png 
-size 1080 1080 -shadows 
-bounces 100 
-jitter 
-filter
```

![07-jitter_filter.png](https://s2.loli.net/2024/05/23/52IQrcjEwkgX4b3.png)







----

<br>

### 使用LOOP曲面细分器 (bunny_1k.obj -> bunny_100w.obj)

```cpp
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

    // Step 3: 更新顶点和三角形数据
    vertices = newVertices;
    triangles = newTriangles;
}
```

#### 06_bunny_1k.txt

```sh
./build/a2 
-input data/scene06_bunny_1k.txt  
-output output/06_1k.png 
-size 1080 1080
-bounces 100 -filter
```
![06_1k.png](https://s2.loli.net/2024/05/23/HPVgi4KWOLNetBZ.png)

#### 06_bunny_100w.txt

```sh
./build/a2 
-input data/scene06_bunny_100w.txt  
-output output/06_100w.png 
-size 1080 1080 
-bounces 100 -filter
```
![06_100w.png](https://s2.loli.net/2024/05/23/csXSI53C6jLf1Gg.png)

