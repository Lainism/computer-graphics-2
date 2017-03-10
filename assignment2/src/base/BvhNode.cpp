// solution code removed
#include <memory>
#include <vector>
#include "rtutil.hpp"
#include "BvhNode.hpp"
#include "RTTriangle.hpp"

FW::Node::Node(std::vector<RTTriangle> &a) : n_tris(a)
{
}

FW::Node::~Node(void)
{
}
