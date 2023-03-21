#pragma once
#include<array>

// scene margins
#define MIN_X -63.0f
#define MAX_X 74.0f
#define MIN_Y -1.2f
#define MAX_Y 30.0f
#define MIN_Z -60.0f
#define MAX_Z 72.0f

namespace gps {

    template<typename T>
    struct BoundingArea {
        BoundingArea() {}
        BoundingArea(T xMin, T xMax, T yMin, T yMax, T zMin, T zMax) : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax) {
        }
        T xMin, xMax, yMin, yMax, zMin, zMax;
    };

    static std::array<BoundingArea<float>, 9> boundaries = {
        BoundingArea<float>(-50.0f,-32.0f,-1.8f,18.0f,42.0f,62.0f), // house1
        BoundingArea<float>(-54.0f,-36.0f,-1.8f,15.0f,-30.0f,-10.0f), // house2
        BoundingArea<float>(-56.0f,-44.0f,-1.8f,25.0f,3.0f,13.0f), // moara1
        BoundingArea<float>(-4.0f,10.0f,-1.8f,25.0f,-28.0f,-13.0f), // moara2
        BoundingArea<float>(-26.0f,-4.0f,-1.8f,0.65f,23.0f,26.0f), // pod
        BoundingArea<float>(-273.0f,-24.0f,-1.8f,0.5f,-1.0f,9.5f), // barca
        BoundingArea<float>(8.0f,50.0f,-1.8f,12.0f,35.0f,67.0f), // teren case langa tractor
        BoundingArea<float>(20.0f,45.0f,-1.8f,12.0f,-56.0f,-36.0f), // teren case langa moara
        BoundingArea<float>(24.0f,41.0f,-1.8f,7,26.0f,34.0f), // teren case langa moara
    };

    template<typename T>
    inline bool insideScene(T x, T y, T z)
    {
        if (y < MIN_Y || MAX_Y < y || x < MIN_X || MAX_X < x || z < MIN_Z || MAX_Z < z)
            return false;
        return true;
    }
}