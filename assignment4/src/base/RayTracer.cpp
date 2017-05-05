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

	// YOUR CODE HERE (R1):
	// Integrate your implementation here.
	Vec2f mapped;

	// Map to 0 - 1
	mapped[0] = uv.x - floor(uv.x);
	mapped[1] = uv.y - floor(uv.y);

	// Scale to resolution
	mapped[0] *= size[0];
	mapped[1] *= size[1];

	return mapped;
}

Mat3f formBasis(const Vec3f& n) {
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
	Mat3f form;
	Vec3f q = n;

	int ind = 0;
	if (abs(q.y) < abs(q[ind])) { ind = 1; }
	if (abs(q.z) < abs(q[ind])) { ind = 2; }

	q[ind] = 1;

	Vec3f t = cross(n, q);
	Vec3f b = cross(n, t);

	form.col(0) = t.normalized();
	form.col(1) = b.normalized();
	form.col(2) = n;

	return form;
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
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
    // After that this is not needed anymore.
    m_rt.reset(new rtlib::RayTracer);
}

RayTracer::~RayTracer()
{
}


void RayTracer::loadHierarchy(const char* filename, std::vector<RTTriangle>& triangles)
{
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
	m_rt->loadHierarchy(filename, triangles);
	m_triangles = &triangles;
}

void RayTracer::saveHierarchy(const char* filename, const std::vector<RTTriangle>& triangles) {
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
    m_rt->saveHierarchy(filename, triangles);
}

void RayTracer::constructHierarchy(std::vector<RTTriangle>& triangles, SplitMode splitMode) {
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
	m_rt->constructHierarchy(triangles, splitMode);
	m_triangles = &triangles;
}


RaycastResult RayTracer::raycast(const Vec3f& orig, const Vec3f& dir) const {
	++m_rayCount;

    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
    return m_rt->raycast(orig, dir);
}


} // namespace FW