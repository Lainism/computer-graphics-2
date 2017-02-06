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
	private:
		Node Build(int i1, int i2, int axis);
		std::vector<RTTriangle> &m_tris;
		std::vector<Node> nodevector;
	};

}
