#pragma once

#include "RTTriangle.hpp"
#include "RaycastResult.hpp"
#include "rtlib.hpp"
#include "rtutil.hpp"
#include "BvhNode.hpp"

#include "base/String.hpp"

#include <vector>
#include <atomic>

namespace FW
{

	class Bvh {
	public:
		Bvh(std::vector<RTTriangle>& triangles, SplitMode splitMode);
		std::vector<Node> nodevector;
		std::vector<RTTriangle> &m_tris;
		std::unique_ptr<Node> root;
		int const nodesPerLeaf = 10;
	private:
		void Build(int i1, int i2, Node& root);
	};

}
