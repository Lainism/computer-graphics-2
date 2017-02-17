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
	bool helper(int i, int j) {}

	Bvh::Bvh(std::vector<RTTriangle>& triangles, SplitMode splitMode): m_tris (triangles)
	{
		Node root = Node();
		root.startPrim = 0;
		root.endPrim = triangles.size()-1;
		nodevector.push_back(root);

		Bvh::Build(0, root.endPrim);
	}

	Node Bvh::Build(int i1, int i2)
	{
		// if empty, return and create node
		// TODO: add stuff to nodevector

		// Create a recursive function here
		// First, let's find out min and max
		Vec3f m_min = m_tris[i1].min();
		Vec3f m_max = m_tris[i1].max();

		// Use correct range
		// for (int i = 0; i < m_tris.size(); i++) {
		for (int i = i1; i < i2; i++) {
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

		Node treenode = Node();
		treenode.startPrim = i1;
		treenode.endPrim = i2;
		auto left = std::make_unique<Node>();
		auto right = std::make_unique<Node>();
		left.get = Bvh::Build(i1, i2 / 2);
		right.get = Bvh::Build(i2 / 2, i2); 

		treenode.leftChild = left;
		treenode.rightChild = right;
	}
}