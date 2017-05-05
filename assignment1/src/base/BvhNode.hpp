#pragma once

#include <memory>
#include <vector>
#include "rtutil.hpp"

namespace FW {

	class Node
	{
	public:
		AABB box; // Axis-aligned bounding box
		int startPrim, endPrim; // Indices in the global list
		bool isLeaf;
		std::vector<std::unique_ptr<Node>> leftChild;
		std::vector<std::unique_ptr<Node>> rightChild;

		Node();
		~Node();
	};

}