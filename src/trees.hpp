#ifndef TREES_HPP
#define TREES_HPP

#include <map>
#include <exception>
#include "types.hpp"
#include "individual.hpp"
#include "../mixlightlib/include/tree.hpp"
#include "randomcore.hpp"

class TreeBalanceProxy{
	np_array<unsigned> tree_;
	unsigned internals_;
	unsigned tips_;
public:
	TreeBalanceProxy(Vec<unsigned> shape) : tree_(shape) {};
	template<typename InfectType>
	friend TreeBalanceProxy treebalanceTree(const Vec<InfectType> & tree);
	np_array<unsigned> getTree() const	{	return tree_;		};
	unsigned getInternals() const		{	return internals_;	};
	unsigned getTips() const			{	return tips_;		};
};


template<typename InfectType>
TreeBalanceProxy treebalanceTree(const Vec<InfectType> & tree){
	// checking that there is only one root
	Individual root;
	auto checkInfectedByExtern = [](const InfectType & i){ return i.inf_loc == -1 && i.inf_ID == -1; };
	unsigned nroots = 0;
	for (auto & i : tree){
		if (checkInfectedByExtern(i)){
			++nroots;
			root = Individual(i.loc, i.ID);
		}
	}
	if (nroots > 1){
		throw std::runtime_error("The simulation has more than one root");
	}
	else if (nroots == 0){
		throw std::logic_error("There is no root, where does the epidemic come from?");
	}
	// creating return array
	unsigned tot_edges = tree.size();
	tot_edges -= nroots;
	TreeBalanceProxy treeTB(Vec<unsigned>({tot_edges,2}));
	// creating references to the individuals
	std::map<Individual,unsigned,IndividualCompare> internal, tips;
	// looping over all the contagions
	for (auto & edge : tree){
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
	internal.erase(root);
	treeTB.internals_ = internal.size() + 1;
	treeTB.tips_ = tips.size();
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
	// looping over all the contagions
	for (auto & edge : tree){
		// performing action only on non-root nodes
		if (!checkInfectedByExtern(edge)){
			Individual infector(edge.inf_loc, edge.inf_ID);
			treeTB.tree_.mutable_at(i,0) = all_ind.at(infector);
			Individual infectee(edge.loc, edge.ID);
			treeTB.tree_.mutable_at(i,1) = all_ind.at(infectee);
			++i;
		}
	}
	return treeTB;
}


class PyTree : public Tree{
public:
	PyTree(const np_array<ParentChild> & array) : Tree(reinterpret_cast<const Edge*>(array.data()), array.shape(0)) {};
	static np_array<ParentChild> getYuleEdges(RNGcore * rng, unsigned nL){
		auto edges = generateYuleEdges(rng->get(), nL);
		return np_array<ParentChild>(edges.size(), reinterpret_cast<ParentChild*>(edges.data()));
	};
	static np_array<ParentChild> getRandomizedEdges(RNGcore * rng, const PyTree & source){
		auto edges = randomizeEdges(rng->get(), source);
		return np_array<ParentChild>(edges.size(), reinterpret_cast<ParentChild*>(edges.data()));
	};
	np_array<unsigned> getDepths(){
		auto depths = computeDepths();
		return np_array<unsigned>(depths.size(), depths.data());
	};
	np_array<double> getProbabilities(){
		auto probs = computeProbabilities();
		return np_array<double>(probs.size(), probs.data());
	};
	np_array<unsigned> getNLeavesSubtree(){
		auto nL_subtree = computeNLeavesSubtree();
		return np_array<unsigned>(nL_subtree.size(), nL_subtree.data());
	};
	double getB2(){
		return computeB2();
	};
	double getB2Norm(){
		return computeB2Norm();
	};
	double getCophenetic(){
		return computeCophenetic();
	};
	double getCopheneticNorm(){
		return computeCopheneticNorm();
	};
	np_array<unsigned> getNChildren() const{
		auto nchildren = computeNChildrenPerNode();
		return np_array<unsigned>(nchildren.size(), nchildren.data());
	}
	np_array<ParentChild> subset(const np_array<Node::ID> & nodes_to_take){
		auto view = nodes_to_take.unchecked<1>();
		std::set<Node::ID> nodesubset({0});
		for (unsigned i = 0; i < nodes_to_take.shape(0); ++i){
			nodesubset.insert(view[i]);
		}
		const auto & nodes = getNodes();
		std::vector<Edge> edges;
		for (Node::ID id : nodesubset){
			if (id == 0)	{	continue;	}
			const Node & node = nodes[id];
			const Node * parent_ptr = node.parent();
			auto iter = nodesubset.find(parent_ptr->id());
			while (iter == nodesubset.end()){
				parent_ptr = parent_ptr->parent();
				iter = nodesubset.find(parent_ptr->id());
			}
			edges.emplace_back(*iter, node.id());
		}
		std::sort(edges.begin(), edges.end());
		std::map<Node::ID,Node::ID> newnames({{0,0}});
		for (auto & edge : edges){
			edge.parent = newnames[edge.parent];
			const auto [it, success] = newnames.insert({edge.child, newnames.size()});
			edge.child = it->second;
		}
		return np_array<ParentChild>(edges.size(), reinterpret_cast<ParentChild*>(edges.data()));
	}
};

#endif