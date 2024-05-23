#ifndef OBJ_TRIANGLE_H
#define OBJ_TRIANGLE_H

#include <array>

// By default counterclockwise winding is front face
struct ObjTriangle {
    ObjTriangle() :
        x{ { 0, 0, 0 } },
        texID{ { 0, 0, 0 } }
    {
    }

    ObjTriangle(int a, int b, int c) :
        x{ { a, b, c } },
        texID{ { 0, 0, 0 } }
    {
    }

    int & operator[](int i) {
        return x[i];
    }

    bool operator==(const ObjTriangle &t) const {
        return x[0] == t.x[0] && x[1] == t.x[1] && x[2] == t.x[2];
    }

    bool operator!=(const ObjTriangle &t) const {
        return x[0] != t.x[0] || x[1] != t.x[1] || x[2] != t.x[2];
    }

    std::array<int, 3> x;
    std::array<int, 3> texID;
};

#endif // OBJ_TRIANGLE_H
