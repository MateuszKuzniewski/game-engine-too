#pragma once
#include <vector>
#include "node.h"

namespace get
{
    class node_world
    {
    public:
        
        node_world(const size_t maxNodes) : _max_nodes(maxNodes)
        {
            _nodes.reserve(_max_nodes);
        }

        ~node_world() = default;


        [[nodiscard]] size_t max_nodes() const { return _max_nodes; }

        std::pair<node&, u32> create_node()
        {
            assert(_nodes.size() < _max_nodes && "Node world is at capacity");
            _nodes.push_back(node{});
            u32 nodeID = _nodes.size();
            return { _nodes[nodeID - 1], nodeID };
        }

        node& get_node(u32 nodeID)
        {
            assert(nodeID > 0 && "nodeID was 0");
            return _nodes[nodeID - 1];
        }

    private:

        std::vector<node> _nodes;

        size_t _max_nodes = 0;
    };
}
