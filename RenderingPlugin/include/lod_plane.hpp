#ifndef LOD_PLANE_HPP
#define LOD_PLANE_HPP

#include <RenderingPlugin.h>

#include <platform.hpp>

#include <vector>
#include <glm/glm.hpp>

struct MyVertex {
    float x, y, z;
    unsigned int color;
	float uvX, uvY;
    
    MyVertex() : x(0.0f), y(0.0f), z(0.0f), color(0), uvX(0.0f), uvY(0.0f) {}
    MyVertex( float x, float y, float z, unsigned int color, float uvX, float uvY ) :
    x(x),
    y(y),
    z(z),
    color(color),
	uvX(uvX),
	uvY(uvY)
    {}
};

class LODPlane {
    public:
		LODPlane( GLuint textureID = 0 );
    
		void setTextureID( GLuint textureID, unsigned int lodLevel );
        void render( float distanceToObserver );

        glm::vec4 centroid() const;
    
    private:
        void subdividePlane( std::vector< MyVertex >& vertices,
                            std::vector< GLubyte>& indices,
                            unsigned int planeFirstVertexIndex );
    
        std::vector< MyVertex > vertices_;
        std::vector< GLubyte > indices_;
		std::vector < unsigned int > textureIDs_;
};

#endif 
// LOD_PLANE_HPP
