#pragma once

#include <memory>
#include <vector>
#include "RTTriangle.hpp"
#include "rtutil.hpp"

namespace FW {

	class Node
	{
	public:
		AABB box; // Axis-aligned bounding box
		int startPrim, endPrim; // Indices in the global list
		bool isLeaf;
		std::vector<RTTriangle> &n_tris;
		std::unique_ptr<Node> leftChild;
		std::unique_ptr<Node> rightChild;

		Node(std::vector<RTTriangle> &a);
		~Node();
	};

}