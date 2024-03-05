#ifndef TREES_HPP
#define TREES_HPP

#include <map>
#include <exception>
#include "types.hpp"
#include "recorder.hpp"
#include "individual.hpp"

np_array<unsigned> consolidateTree(const Vec<const Vec<InfecTree>*> & trees){
	// checking that there is only one root
	Individual root;
	auto checkInfectedByExtern = [](const InfecTree & i){ return i.inf_loc == -1 && i.inf_ID == -1; };
	unsigned nroots = 0;
	for (auto & tree : trees){
		for (auto & i : *tree){
			if (checkInfectedByExtern(i)){
				++nroots;
				root = Individual(i.loc, i.ID);
			}
		}
	}
	if (nroots > 1){
		throw std::runtime_error("The simulation has more than one root");
	}
	else if (nroots == 0){
		throw std::logic_error("There is no root, where does the epidemic come from?");
	}
	// creating return array
	unsigned tot_edges = 0;
	for (auto & tree : trees){
		tot_edges += tree->size();
	}
	tot_edges -= nroots;
	np_array<unsigned> array(Vec<unsigned>({tot_edges,2}));
	// creating references to the individuals
	std::map<Individual,unsigned,IndividualCompare> internal, tips;
	// looping over all the patches
	for (auto & tree : trees){
		// looping over all the contagions
		for (auto & edge : *tree){
			// performing action only on non-root nodes
			if (!checkInfectedByExtern(edge)){
				// adding the infector to the internal pool
				// and removing it from the tips, if present
				Individual infector(edge.inf_loc, edge.inf_ID);
				internal[infector] = 0;
				auto it = tips.find(infector);
				if (it != tips.end()){
					tips.erase(it);
				}
				// adding the infectee to the tips pool if it is not internal already
				Individual infectee(edge.loc, edge.ID);
				if (!internal.contains(infectee)){
					tips[infectee] = 0;
				}
			}
		}
	}
	internal.erase(root);
//////////////////
py::print("internal", internal.size(), "tips", tips.size());
//////////////////
	// creating a unique pool to map the individuals, setting new names for the nodes:
	// considering n (n° of tips) and m (total number of individuals involved)
	// tips: [1, n] // root : n+1 // internal : [n+2, m]
	std::map<Individual,unsigned,IndividualCompare> all_ind;
	{
		unsigned i = 1;
		for (auto & ind : tips){
			ind.second = i;
			++i;
		}
		all_ind.merge(tips);
		all_ind[root] = i;
		++i;
		for (auto & ind : internal){
			ind.second = i;
			++i;
		}
		all_ind.merge(internal);
		if (internal.size()+tips.size() != 0){
			throw std::logic_error("Not all tips and internals have been merged");
		}
	}
	// converting all the values
	unsigned i = 0;
	// looping over all the patches
	for (auto & tree : trees){
		// looping over all the contagions
		for (auto & edge : *tree){
			// performing action only on non-root nodes
			if (!checkInfectedByExtern(edge)){
				Individual infector(edge.inf_loc, edge.inf_ID);
				array.mutable_at(i,0) = all_ind.at(infector);
				Individual infectee(edge.loc, edge.ID);
				array.mutable_at(i,1) = all_ind.at(infectee);
				++i;
			}
		}
	}
	return array;
}

#endif