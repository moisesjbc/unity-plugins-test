#include <lod_plane.hpp>

INITIALIZE_EASYLOGGINGPP

LODPlane::LODPlane( GLuint textureID ) :
	textureIDs_( 3, textureID )
{
    // A plane.
    MyVertex srcVertices[] =
    {
        MyVertex( -1.5f, 1.0f, -1.5f, 0xFFff0000, 0.0f, 0.0f ),
        MyVertex( 1.5f, 1.0f, -1.5f, 0xFF00ff00, 1.0f, 0.0f ),
        MyVertex( 1.5f, 1.0f, 1.5f, 0xFF0000ff, 1.0f, 1.0f ),
        MyVertex( -1.5f, 1.f, 1.5f, 0xFF0f0f0f, 0.0f, 1.0f )
    };
    
    // Copy original plane to vertices vector
    for( int i = 0; i < 4; i++ ){
        vertices_.push_back( srcVertices[i] );
    }
    // First triangle.
    indices_.push_back( 2 );
    indices_.push_back( 1 );
    indices_.push_back( 0 );
    // Second triangle.
    indices_.push_back( 3 );
    indices_.push_back( 2 );
    indices_.push_back( 0 );
    
    // Subdivide plane (2).
    subdividePlane( vertices_, indices_, 0 );
    
    // Subdivide plane (3).
    subdividePlane( vertices_, indices_, 6 );
    subdividePlane( vertices_, indices_, 12 );
    subdividePlane( vertices_, indices_, 18 );
    subdividePlane( vertices_, indices_, 24 );
}


void LODPlane::setTextureID( GLuint textureID, unsigned int lodLevel )
{
	textureIDs_.at( lodLevel ) = textureID;
}


void LODPlane::render( float distanceToObserver )
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Vertex layout.
    const int stride = 3*sizeof(float) + sizeof(unsigned int) + 2 * sizeof( float );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLfloat*)vertices_.data());
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLfloat*)vertices_.data() + 3);

	glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, stride, (const GLbyte*)vertices_.data() + 3 * sizeof(GLfloat) + sizeof(unsigned int) );

	GLint currentShaderProgram;

	// Get current shader program id.
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentShaderProgram);

	// Get location of sampler in shader.
	const GLint samplerShaderLocation = glGetUniformLocation(currentShaderProgram, "textureSampler");
	LOG(INFO) << "samplerShaderLocation: " << samplerShaderLocation << std::endl;

	// Connect sampler to texture unit 0.
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(samplerShaderLocation, 0);

    // Draw a version of the plane or another depending on the distance between
    // the camera and the plane.
    const unsigned int N_INDICES_PER_PLANE = 6;
    if( distanceToObserver > 3.0f ){
		glBindTexture(GL_TEXTURE_2D, textureIDs_.at( 0 ) );
        glDrawElements( GL_TRIANGLES, N_INDICES_PER_PLANE, GL_UNSIGNED_BYTE, indices_.data() );
    }else if( distanceToObserver > 2.0f ){
		glBindTexture(GL_TEXTURE_2D, textureIDs_.at( 1 ) );
        glDrawElements( GL_TRIANGLES, 4 * N_INDICES_PER_PLANE, GL_UNSIGNED_BYTE, indices_.data() + N_INDICES_PER_PLANE );
    }else{
		glBindTexture(GL_TEXTURE_2D, textureIDs_.at( 2 ) );
        glDrawElements( GL_TRIANGLES, 16 * N_INDICES_PER_PLANE, GL_UNSIGNED_BYTE, indices_.data() + 5 * N_INDICES_PER_PLANE );
    }
}


glm::vec4 LODPlane::centroid() const
{
    glm::vec4 centroid( 0.0f );

    for( unsigned int i = 0; i < 4; i++ ){
        centroid.x += vertices_[i].x;
        centroid.y += vertices_[i].y;
        centroid.z += vertices_[i].z;
    }

    centroid /= 3.0f;
    centroid[3] = 1.0f;

    return centroid;
}


void LODPlane::subdividePlane( std::vector< MyVertex >& vertices,
                              std::vector< GLubyte>& indices,
                              unsigned int planeFirstVertexIndex )
{
    // Retrieve all the indices of the plane to be subdivided.
    const GLubyte planeVertexIndices[4] =
    {
        indices[planeFirstVertexIndex + 2],
        indices[planeFirstVertexIndex + 1],
        indices[planeFirstVertexIndex],
        indices[planeFirstVertexIndex + 3]
    };
    
    // Retrieve all the vertices of the plane being subdivided.
    MyVertex planeVertices[4] =
    {
        vertices[planeVertexIndices[0]],
        vertices[planeVertexIndices[1]],
        vertices[planeVertexIndices[2]],
        vertices[planeVertexIndices[3]]
    };
    
    // Compute plane centroid and introduce it in the vertices vector.
    MyVertex planeCentroid;
    for( int i = 0; i < 4; i++ )
    {
        planeCentroid.x += planeVertices[i].x;
        planeCentroid.y += planeVertices[i].y;
        planeCentroid.z += planeVertices[i].z;
        planeCentroid.color += planeVertices[i].color;
		planeCentroid.uvX += planeVertices[i].uvX;
		planeCentroid.uvY += planeVertices[i].uvY;
    }
    planeCentroid.x /= 4.0f;
    planeCentroid.y /= 4.0f;
    planeCentroid.z /= 4.0f;
    planeCentroid.color /= 4;
	planeCentroid.uvX /= 4.0f;
	planeCentroid.uvY /= 4.0f;
    vertices.push_back( planeCentroid );
    const GLubyte planeCentroidIndex = static_cast<GLubyte>( vertices.size() - 1 );
    
    // Compute the middle vertices of the plane and push them into the vertices vector.
    GLubyte midleVertexIndices[4];
    for( int i = 0; i < 4; i++ ){
        const MyVertex& currentVertex = planeVertices[i];
        const MyVertex& nextVertex = planeVertices[(i+1)%4];
        const MyVertex middleVertex(
                                    ( currentVertex.x + nextVertex.x ) / 2.0f,
                                    ( currentVertex.y + nextVertex.y ) / 2.0f,
                                    ( currentVertex.z + nextVertex.z ) / 2.0f,
                                    0xFFffffff,
									(currentVertex.uvX + nextVertex.uvX) / 2.0f,
									(currentVertex.uvY + nextVertex.uvY) / 2.0f
                                    );
        
        vertices.push_back( middleVertex );
        midleVertexIndices[i] = static_cast<GLubyte>( vertices.size() - 1 );
    }
    
    // Now we have all the new vertices into vertices. Let's define the
    // subplanes by feeding indices vector with new indices.
    
    // Subplane 0
    indices.push_back( planeCentroidIndex );
    indices.push_back( midleVertexIndices[0] );
    indices.push_back( planeVertexIndices[0] );

    indices.push_back( midleVertexIndices[3] );
    indices.push_back( planeCentroidIndex );
    indices.push_back( planeVertexIndices[0] );

    
    // Subplane 1
    indices.push_back( midleVertexIndices[1] );
    indices.push_back( planeVertexIndices[1] );
    indices.push_back( midleVertexIndices[0] );

    indices.push_back( planeCentroidIndex );
    indices.push_back( midleVertexIndices[1] );
    indices.push_back( midleVertexIndices[0] );

    
    // Subplane 2
    indices.push_back( planeVertexIndices[2] );
    indices.push_back( midleVertexIndices[1] );
    indices.push_back( planeCentroidIndex );

    indices.push_back( midleVertexIndices[2] );
    indices.push_back( planeVertexIndices[2] );
    indices.push_back( planeCentroidIndex );

    
    // Subplane 3
    indices.push_back( midleVertexIndices[2] );
    indices.push_back( planeCentroidIndex );
    indices.push_back( midleVertexIndices[3] );

    indices.push_back( planeVertexIndices[3] );
    indices.push_back( midleVertexIndices[2] );
    indices.push_back( midleVertexIndices[3] );
}
