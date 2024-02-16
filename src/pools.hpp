#ifndef POOLS_HPP
#define POOLS_HPP

#include <concepts>
#include "types.hpp"
#include "pools/mixpool.hpp"
#include "pools/indivactive.hpp"
#include "pools/indivpassive.hpp"
#include "pools/indivdiff.hpp"
#include "pools/indivbind.hpp"


template<typename T>
concept Pool = requires(typename T::Active a,
						typename T::Passive p,
						typename T::Diff d,
						PatchID pid,
						unsigned u,
						double dbl,
						RNGcore * rng){
	new T::Passive(pid);		new T::Passive(pid,u);
	p.size();
	d = p.generate(u);			d = p.generate(d);
	new T::Active(pid);
	a.size();
	{a.getPhi()} -> std::same_as<double>;
	d = a.sample(rng, u);		d = a.sampleWithReplacement(rng, u);
	new T::Diff(pid);
	d.size();
	d += d;
	d.moveFromTo(p,a);			d.moveFromTo(a,p);			d.moveFromTo(a,a);
	d.clear();
};


struct Mix{
	using Active = MixPool;
	using Passive = MixPool;
	using Diff = MixPool;
};


struct Individuals{
	using Active = IndivActive;
	using Passive = IndivPassive;
	using Diff = IndivDiff;
};

#endif