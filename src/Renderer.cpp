#include "Renderer.h"

#include "ArgParser.h"
#include "Camera.h"
#include "Image.h"
#include "Ray.h"
#include "VecUtils.h"

#include <limits>
#include <random>

Renderer::Renderer(const ArgParser& args) : _args(args), _scene(args.input_file) {}

// 主体渲染循环
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

            // 测试阴影
            if (_args.shadows) {
                Hit h_test;  // 阴影测试交点
                Ray r_test = {p + tolight * 0.001f, tolight.normalized()};  // 阴影测试光线
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
