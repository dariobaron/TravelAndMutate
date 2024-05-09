#ifndef QUICKANALYSIS_HPP
#define QUICKANALYSIS_HPP

#include <map>
#include "types.hpp"
#include "haplotypes.hpp"

struct IdDepth{
	unsigned id, depth;
};
np_array<IdDepth> computeDepths(const np_array<ParentChild> & tree){
	std::map<unsigned,unsigned> depths;
	auto view_tree = tree.unchecked<1>();
	for (unsigned i = 0; i < tree.shape(0); ++i){
		auto it = depths.find(view_tree[i].parent);
		if (it == depths.end())	{	depths[view_tree[i].child] = 0;	}
		else					{	depths[view_tree[i].child] = 1 + depths[view_tree[i].parent];	}
	}
	np_array<IdDepth> arr_depths(depths.size());
	auto view_dept = arr_depths.mutable_unchecked<1>();
	unsigned idx = 0;
	for (auto & pair : depths){
		view_dept[idx].id = pair.first;
		view_dept[idx].depth = pair.second;
		++idx;
	}
	return arr_depths;
}


#endif