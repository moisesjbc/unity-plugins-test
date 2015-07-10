#ifndef LOD_PLANE_HPP
#define LOD_PLANE_HPP

struct MyVertex {
    float x, y, z;
    unsigned int color;
    
    MyVertex() : x(0.0f), y(0.0f), z(0.0f), color(0) {}
    MyVertex( float x, float y, float z, unsigned int color ) :
    x(x),
    y(y),
    z(z),
    color(color)
    {}
};

class LODPlane {
};

#endif 
// LOD_PLANE_HPP