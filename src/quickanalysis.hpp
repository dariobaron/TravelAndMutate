#ifndef QUICKANALYSIS_HPP
#define QUICKANALYSIS_HPP

#include <exception>
#include "types.hpp"
#include "haplotypes.hpp"

struct IdDepth{
	unsigned id, depth;
};
np_array<IdDepth> computeDepths(const np_array<ParentChild> & tree){
	np_array<IdDepth> depths(tree.shape(0));
	auto view_d = depths.mutable_unchecked<1>();
	for (unsigned i = 0; i < depths.shape(0); ++i){
		view_d[i].id = i;
		view_d[i].depth = 0;
	}
	auto view_t = tree.unchecked<1>();
	for (unsigned i = 0; i < tree.shape(0); ++i){
		auto branch = view_t[i];
		if (i != 0){
			if ((branch.parent >= branch.child) || (branch.child > i)){
				throw std::runtime_error("The tree is ill-formed!");
			}
		}
		if (branch.parent < depths.shape(0)){
			view_d[branch.child].depth = 1 + view_d[branch.parent].depth;
		}
	}
	return depths;
}

struct IdChildren{
	unsigned id, children;
};
np_array<IdChildren> computeChildren(const np_array<ParentChild> & tree){
	np_array<IdChildren> children(tree.shape(0));
	auto view_c = children.mutable_unchecked<1>();
	for (unsigned i = 0; i < children.shape(0); ++i){
		view_c[i].id = i;
		view_c[i].children = 0;
	}
	auto view_t = tree.unchecked<1>();
	for (unsigned i = 0; i < tree.shape(0); ++i){
		auto branch = view_t[i];
		if (i != 0){
			if ((branch.parent >= branch.child) || (branch.child > i)){
				throw std::runtime_error("The tree is ill-formed!");
			}
		}
		if (branch.parent < children.shape(0)){
			++view_c[branch.parent].children;
		}
	}
	return children;
}


#endif