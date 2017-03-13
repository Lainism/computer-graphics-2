#define _CRT_SECURE_NO_WARNINGS

#include "base/Defs.hpp"
#include "base/Math.hpp"
#include "RayTracer.hpp"
#include <stdio.h>
#include "rtIntersect.inl"
#include <fstream>
#include "Bvh.hpp"

#include "rtlib.hpp"


// Helper function for hashing scene data for caching BVHs
extern "C" void MD5Buffer( void* buffer, size_t bufLen, unsigned int* pDigest );


namespace FW
{


Vec2f getTexelCoords(Vec2f uv, const Vec2i size)
{

	// YOUR CODE HERE (R1):
	// Integrate your implementation here.

	// Not implemented in first round
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

	// Not implemented in first round
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
    //m_rt.reset(new rtlib::RayTracer);
}

RayTracer::~RayTracer()
{
}


void RayTracer::loadHierarchy(const char* filename, std::vector<RTTriangle>& triangles)
{
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.

	// Not implemented in first round
	//m_rt->loadHierarchy(filename, triangles);
	m_triangles = &triangles;
}

void RayTracer::saveHierarchy(const char* filename, const std::vector<RTTriangle>& triangles) {
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
	// Not implemented in first round

    //m_rt->saveHierarchy(filename, triangles);
}

bool RayTracer::check_intersect(AABB& box, const Vec3f& orig, const Vec3f& dir, float& tmin) const {

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

	tmin = xmin;

	return true;
}

RaycastResult RayTracer::recursiveHelper(const Vec3f& orig, const Vec3f& dir, int& imin, float& tmin, Node* root) const {
	AABB left_box;
	AABB right_box;

	float umin = 0.0f, vmin = 0.0f;

	RaycastResult castresult;
	RaycastResult h1;
	RaycastResult h2;
	bool leafhit = false;
	bool both = false;

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
				leafhit = true;
			}
		}
	}
	else {

		// What if there is only one node? - Impossible
		AABB left_box = root->leftChild->box;
		AABB right_box = root->rightChild->box;

		// Compare both
		// if tmin < xmin -> skip
		float xmin1;
		float xmin2;
		bool t1 = check_intersect(left_box, orig, dir, xmin1);
		bool t2 = check_intersect(right_box, orig, dir, xmin2);
		Node* child1 = root->leftChild.get();
		Node* child2 = root->rightChild.get();
		float bigger = xmin2;

		if (xmin1 > xmin2) {
			std::swap(child1, child2);
			std::swap(t1, t2);
			bigger = xmin1;
		}

		// Calculate if the ray hits children based on origo and direction
		if (t1) {
			h1 = recursiveHelper(orig, dir, imin, tmin, child1);
		}

		if (t2 && (bigger < tmin)) {
			h2 = recursiveHelper(orig, dir, imin, tmin, child2);
			both = true;
		}
	}

	// Changed triangles to using tree's local list, since it's sorted the same way as Bvh
	if (leafhit) {
		castresult = RaycastResult(&tree->m_tris[imin], tmin, umin, vmin, orig + tmin*dir, orig, dir);
	}
	else {
		if (!both || (h1.t < h2.t)) {
			castresult = h1;
		}
		else {
			castresult = h2;
		}
	}

	return castresult;
}

void RayTracer::constructHierarchy(std::vector<RTTriangle>& triangles, SplitMode splitMode) {
    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
	//m_rt->constructHierarchy(triangles, splitMode);
	tree = new Bvh(triangles, splitMode);
	m_triangles = &triangles;
}


RaycastResult RayTracer::raycast(const Vec3f& orig, const Vec3f& dir) const {
	++m_rayCount;

    // YOUR CODE HERE (R1):
    // Integrate your implementation here.
    //return m_rt->raycast(orig, dir);

	Node* root = (*tree).root.get();
	int imin = -1;
	float tmin = 1.0f;

	RaycastResult castresult = RayTracer::recursiveHelper(orig, dir, imin, tmin, root);

	return castresult;
}


} // namespace FW