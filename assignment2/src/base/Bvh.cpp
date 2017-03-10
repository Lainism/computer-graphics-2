#include "RTTriangle.hpp"
#include "RaycastResult.hpp"
#include "rtlib.hpp"
#include "rtutil.hpp"
#include "BvhNode.hpp"

#include "base/String.hpp"

#include <algorithm>
#include <vector>
#include <atomic>
#include "Bvh.hpp"

namespace FW
{

	Bvh::Bvh(std::vector<RTTriangle>& triangles, SplitMode splitMode) : m_tris(triangles)
	{
		root = std::make_unique<Node>();
		root->isLeaf = false;
		root->startPrim = 0;
		root->endPrim = triangles.size() - 1;
		Bvh::Build(root->startPrim, root->endPrim, *root);
	}

	void Bvh::Build(int i1, int i2, Node& root)
	{
		// Create a recursive function here
		// First, let's find out min and max

		Vec3f m_min = m_tris[i1].min();
		Vec3f m_max = m_tris[i1].max();

		// Use correct range
		for (int i = i1; i <= i2; i++) {
			m_min = min(m_min, m_tris[i].min());
			m_max = max(m_max, m_tris[i].max());
		}

		// Choose right axis
		float x_dif = abs(m_max[0] - m_min[0]);
		float y_dif = abs(m_max[1] - m_min[1]);
		float z_dif = abs(m_max[2] - m_min[2]);

		int axis = 0;

		if (y_dif > x_dif) axis = 1;
		if (z_dif > y_dif && z_dif > x_dif) axis = 2;

		// Sort tris according to chosen axis
		std::sort(m_tris.begin() + i1, m_tris.begin() + i2, [axis](const RTTriangle & a, const RTTriangle & b) -> bool { return a.centroid()[axis] > b.centroid()[axis]; });

		root.startPrim = i1;
		root.endPrim = i2;

		AABB hitbox = AABB();
		hitbox.min = m_min;
		hitbox.max = m_max;
		root.box = hitbox;

		int delta = i2 - i1;

		root.leftChild = std::make_unique<Node>();
		root.rightChild = std::make_unique<Node>();

		// if empty, return and create node
		// add stuff to nodevector
		if (delta > nodesPerLeaf) {
			Bvh::Build(i1, delta / 2 + i1, *(root.leftChild));
			Bvh::Build(delta / 2 + i1 + 1, i2, *(root.rightChild));

			root.isLeaf = false;
		}
		else {
			for (int i = 0; i+i1 <= i2; i++) {
				root.n_tris.push_back(m_tris[i1+i]);
			}
			root.isLeaf = true;
		}
	}
}