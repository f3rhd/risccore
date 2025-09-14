#pragma once
#include <vector>
#include <string>
struct interference_node_t {
	std::string id;
	std::vector<interference_node_t*> edges;
};