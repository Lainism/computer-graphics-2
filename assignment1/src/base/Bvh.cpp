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

	Bvh::Bvh(std::vector<RTTriangle>& triangles, SplitMode splitMode): m_tris (triangles)
	{
		Bvh::Build(0, triangles.size() - 1);
	}

	Node Bvh::Build(int i1, int i2)
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
		
		std::unique_ptr<Node> node_ptr(new Node);
		
		(*node_ptr).startPrim = i1;
		(*node_ptr).endPrim = i2;

		AABB hitbox = AABB();
		hitbox.min = m_min;
		hitbox.max = m_max;
		(*node_ptr).box = hitbox;

		int delta = i2 - i1;

		(*node_ptr).leftChild = std::make_unique<Node>();
		(*node_ptr).rightChild = std::make_unique<Node>();

		// if empty, return and create node
		// add stuff to nodevector
		if (i1 < i2) {
			(*node_ptr).leftChild =  std::move(Bvh::Build(i1, delta / 2 + i1));
			(*node_ptr).rightChild =  std::move(Bvh::Build(delta / 2 + i1 + 1, i2));
		}
		else {
			(*node_ptr).leftChild = NULL;
			(*node_ptr).rightChild = NULL;
		}

		nodevector.push_back(std::move(node_ptr));
		return (*node_ptr);
	}
}