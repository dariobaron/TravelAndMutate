#ifndef QUICKANALYSIS_HPP
#define QUICKANALYSIS_HPP

#include "types.hpp"
#include "haplotypes.hpp"

struct IdDepth{
	unsigned id, depth;
};

np_array<IdDepth> computeDepths(const Haplotypes * dealer){
	auto parents = dealer->getParents();
	np_array<IdDepth> depths(parents.size());
	auto view = depths.mutable_unchecked<1>();
	for (unsigned i = 0; i < parents.size(); ++i){
		view[i].id = i;
		if (parents[i] < parents.size()){
			view[i].depth = view[parents[i]].depth + 1;
		}
		else{
			view[i].depth = 0;
		}
	}
	return depths;
}


#endif