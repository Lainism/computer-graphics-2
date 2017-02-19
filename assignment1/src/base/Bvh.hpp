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
		std::vector<std::unique_ptr<Node>> nodevector;
		std::vector<RTTriangle> &m_tris;
	private:
		Node Build(int i1, int i2);
	};

}
