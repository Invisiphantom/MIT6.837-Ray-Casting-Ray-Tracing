#include "ArgParser.h"

#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>

ArgParser::ArgParser(int argc, const char *argv[])
{
    defaultValues();

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-input")) // 输入文件
        {
            i++;
            assert(i < argc);
            input_file = argv[i];
        }
        else if (!strcmp(argv[i], "-output")) // 输出文件
        {
            i++;
            assert(i < argc);
            output_file = argv[i];
        }
        else if (!strcmp(argv[i], "-normals")) // 法线图
        {
            i++;
            assert(i < argc);
            normals_file = argv[i];
        }
        else if (!strcmp(argv[i], "-size")) // 图片大小
        {
            i++;
            assert(i < argc);
            width = atoi(argv[i]);
            i++;
            assert(i < argc);
            height = atoi(argv[i]);
        }

        else if (!strcmp(argv[i], "-depth")) // 生成深度图
        {
            i++;
            assert(i < argc);
            depth_min = (float)atof(argv[i]);
            i++;
            assert(i < argc);
            depth_max = (float)atof(argv[i]);
            i++;
            assert(i < argc);
            depth_file = argv[i];
        }
        else if (!strcmp(argv[i], "-bounces")) // 光追最大递归深度
        {
            i++;
            assert(i < argc);
            bounces = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-shadows")) // 投射阴影
        {
            shadows = true;
        }

        // supersampling
        else if (strcmp(argv[i], "-jitter") == 0)
        {
            jitter = true;
        }
        else if (strcmp(argv[i], "-filter") == 0)
        {
            filter = true;
        }
        else
        {
            printf("Unknown command line argument %d: '%s'\n", i, argv[i]);
            exit(1);
        }
    }

    std::cout << "Args:\n";
    std::cout << "- input: " << input_file << std::endl;
    std::cout << "- output: " << output_file << std::endl;
    std::cout << "- depth_file: " << depth_file << std::endl;
    std::cout << "- normals_file: " << normals_file << std::endl;
    std::cout << "- width: " << width << std::endl;
    std::cout << "- height: " << height << std::endl;
    std::cout << "- depth_min: " << depth_min << std::endl;
    std::cout << "- depth_max: " << depth_max << std::endl;
    std::cout << "- bounces: " << bounces << std::endl;
    std::cout << "- shadows: " << shadows << std::endl;
}

void ArgParser::defaultValues()
{
    // rendering output
    input_file = "";
    output_file = "";
    depth_file = "";
    normals_file = "";
    width = 600;
    height = 600;
    stats = 0;

    // rendering options
    depth_min = 0;
    depth_max = 1;
    bounces = 0;
    shadows = false;

    // sampling
    jitter = false;
    filter = false;
}
