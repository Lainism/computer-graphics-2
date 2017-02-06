#pragma once

#include <memory>
#include "rtutil.hpp"

namespace FW {

	struct Node
	{
		AABB box; // Axis-aligned bounding box
		int startPrim, endPrim; // Indices in the global list
		std::unique_ptr<Node> leftChild;
		std::unique_ptr<Node> rightChild;
	};

}