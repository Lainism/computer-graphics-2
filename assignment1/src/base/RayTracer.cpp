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

	tree = new Bvh(triangles, splitMode);

	
	//int m_size = triangles.size();
	//m_tris = triangles;
	/*
	if (m_size == 1) {
		m_triangles = &triangles;
		return;
	}

	triangles[1].max();
	*/
	//constructHierarchy();

    m_triangles = &triangles;
	
}

bool RayTracer::check_intersect(AABB& box, const Vec3f& orig, const Vec3f& dir) const {

	// There might be a problem here...

	auto vmin = box.min;
	auto vmax = box.max;

	auto xmin = (vmin[0] - orig[0]) / dir[0];
	auto xmax = (vmax[0] - orig[0]) / dir[0];

	auto ymin = (vmin[1] - orig[1]) / dir[1];
	auto ymax = (vmax[1] - orig[1]) / dir[1];

	auto zmin = (vmin[2] - orig[2]) / dir[2];
	auto zmax = (vmax[2] - orig[2]) / dir[2];

	if (xmin > xmax) { std::swap(xmin, xmax); }
	if (ymin > ymax) { std::swap(ymin, ymax); }
	if (zmin > zmax) { std::swap(zmin, zmax); }

	if ((xmin > ymax) || (ymin > xmax)) { return false; }

	if (ymin > xmin) { xmin = ymin; }
	if (ymax < xmax) { xmax = ymax; }

	if ((xmin > zmax) || (zmin > xmax)) { return false; }

	if (zmin > xmin) { xmin = zmin; }
	if (zmax < xmax) { xmax = zmax; }

	return true;
}

RaycastResult RayTracer::raycast(const Vec3f& orig, const Vec3f& dir) const {
	++m_rayCount;

    // YOUR CODE HERE (R1):
    // This is where you hierarchically traverse the tree you built!
    // You can use the existing code for the leaf nodes.

	// get root
	// root get always pushed in the list last
	//int index = (*tree).nodevector.size()-1;
	Node* root = (*tree).root.get();
	AABB left_box;
	AABB right_box;

	float tmin = 1.0f, umin = 0.0f, vmin = 0.0f;
	int imin = -1;

	RaycastResult castresult;

	while (true) {
		if (root->isLeaf) {
			// We're in a leaf node
			float t, u, v;
			if ((*tree).m_tris[root->startPrim].intersect_woop(orig, dir, t, u, v))
			{
				if (t > 0.0f && t < tmin)
				{
					imin = root->startPrim;
					tmin = t;
					umin = u;
					vmin = v;
				}
			}
			break;
		}

		// What if there is only one node? - Impossible
		AABB left_box = root->leftChild->box;
		AABB right_box = root->rightChild->box;

		// Calculate if the ray hits children based on origo and direction
		if (check_intersect(left_box, orig, dir)) {
			root = root->leftChild.get();
		}
		else if (check_intersect(right_box, orig, dir)) {
			root = root->rightChild.get();
		}
		else {
			// Ray missed the hitboxes
			break;
		}
	}

    // Naive loop over all triangles.
	/*
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
	*/

	// Changed triangles to using tree's local list, since it's sorted the same way as Bvh
    if (imin != -1) {
        castresult = RaycastResult(&tree->m_tris[imin], tmin, umin, vmin, orig + tmin*dir, orig, dir);
    }
    return castresult;
}


} // namespace FW