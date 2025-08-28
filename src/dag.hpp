#pragma once

#include <map>
#include <set>
#include <stack>
#include <exception>
#include <pybind11/stl.h>
#include "types.hpp"


std::map<unsigned,unsigned> computeSubnetSizes(const np_array<ParentChild> & edgelist, std::vector<unsigned> roots){
	std::map<unsigned,std::set<unsigned>> children;
	for (unsigned i = 0; i < edgelist.shape(0); ++i){
		auto edge = edgelist.at(i);
		children[edge.parent];
		children[edge.child];
	}
	for (unsigned i = 0; i < edgelist.shape(0); ++i){
		auto edge = edgelist.at(i);
		children[edge.parent].insert(edge.child);
	}

	std::map<unsigned,std::set<unsigned>> subtrees;
	for (auto & [id,kids] : children){
		if (kids.empty()) {
			subtrees[id];
		}
	}
	std::stack<unsigned> to_process;
	for (unsigned root : roots){
		to_process.push(root);
	}
	while (!to_process.empty()){
		auto id = to_process.top();
		while (subtrees.contains(id)){
			to_process.pop();
			id = to_process.top();
		}
		bool all_children_processed = true;
		for (auto child : children.at(id)){
			if (!subtrees.contains(child)){
				all_children_processed = false;
				to_process.push(child);
			}
		}
		if (all_children_processed){
			to_process.pop();
			std::set<unsigned> subtree;
			for (auto child : children.at(id)){
				std::set<unsigned> subsubtree = subtrees.at(child);
				subtree.insert(subsubtree.begin(), subsubtree.end());
				subtree.insert(child);
			}
			subtrees[id] = subtree;
		}
	}

	std::map<unsigned,unsigned> sizes;
	for (auto & pair : subtrees){
		sizes[pair.first] = pair.second.size();
	}
	return sizes;
}