#define _CRT_SECURE_NO_WARNINGS

#include "base/Defs.hpp"
#include "base/Math.hpp"
#include "RayTracer.hpp"
#include <stdio.h>
#include "rtIntersect.inl"
#include <fstream>

#include "rtlib.hpp"


// Helper function for hashing scene data for caching BVHs
extern "C" void MD5Buffer( void* buffer, size_t bufLen, unsigned int* pDigest );


namespace FW
{


Vec2f getTexelCoords(Vec2f uv, const Vec2i size)
{

	// YOUR CODE HERE (R3):
	// Get texel indices of texel nearest to the uv vector. Used in texturing.
	// UV coordinates range from negative to positive infinity. First map them
	// to a range between 0 and 1 in order to support tiling textures, then
	// scale the coordinates by image resolution and find the nearest pixel.
	return Vec2f();
}

Mat3f formBasis(const Vec3f& n) {
    // YOUR CODE HERE (R4):
    return Mat3f();
}


String RayTracer::computeMD5( const std::vector<Vec3f>& vertices )
{
    unsigned char digest[16];
    MD5Buffer( (void*)&vertices[0], sizeof(Vec3f)*vertices.size(), (unsigned int*)digest );

    // turn into string
    char ad[33];
    for ( int i = 0; i < 16; ++i )
        ::sprintf( ad+i*2, "%02x", digest[i] );
    ad[32] = 0;

    return FW::String( ad );
}


// --------------------------------------------------------------------------


RayTracer::RayTracer()
{
}

RayTracer::~RayTracer()
{
}


void RayTracer::loadHierarchy(const char* filename, std::vector<RTTriangle>& triangles)
{
    // YOUR CODE HERE (R2):
    m_triangles = &triangles;
}

void RayTracer::saveHierarchy(const char* filename, const std::vector<RTTriangle>& triangles) {
    // YOUR CODE HERE (R2)
}

void RayTracer::constructHierarchy(std::vector<RTTriangle>& triangles, SplitMode splitMode) {
    // YOUR CODE HERE (R1):
    m_triangles = &triangles;
}


RaycastResult RayTracer::raycast(const Vec3f& orig, const Vec3f& dir) const {
	++m_rayCount;

    // YOUR CODE HERE (R1):
    // This is where you hierarchically traverse the tree you built!
    // You can use the existing code for the leaf nodes.

    float tmin = 1.0f, umin = 0.0f, vmin = 0.0f;
    int imin = -1;

    RaycastResult castresult;

    // Naive loop over all triangles.
    for ( size_t i = 0; i < m_triangles->size(); ++i )
    {
        float t, u, v;
        if ( (*m_triangles)[i].intersect_woop( orig, dir, t, u, v ) )
        {
            if ( t > 0.0f && t < tmin )
            {
                imin = i;
                tmin = t;
                umin = u;
                vmin = v;
            }
        }
    }

    if (imin != -1) {
        castresult = RaycastResult(&(*m_triangles)[imin], tmin, umin, vmin, orig + tmin*dir, orig, dir);
    }
    return castresult;
}


} // namespace FW