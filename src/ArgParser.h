#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>

class ArgParser {
public:
    ArgParser(int argc, const char *argv[]);

    // ==============
    // REPRESENTATION
    // All public! (no accessors).

    // rendering output
    std::string input_file; // 输入文件
    std::string output_file; // 输出文件
    std::string depth_file; // 深度文件
    std::string normals_file; // 法线文件
    int width; // 图片宽度
    int height; // 图片高度
    int stats;

    // rendering options
    float depth_min;
    float depth_max;
    int bounces; // 光追最大递归深度
    bool shadows; // 是否投射阴影

    // supersampling
    bool jitter;
    bool filter;

private:
    void defaultValues();
};

#endif // ARG_PARSER_H
