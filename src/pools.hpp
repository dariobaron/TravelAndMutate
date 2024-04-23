#ifndef POOLS_HPP
#define POOLS_HPP

#include <concepts>
#include "types.hpp"
#include "pools/mixpool.hpp"
#include "pools/individuals/indivactive.hpp"
#include "pools/individuals/indivpassive.hpp"
#include "pools/individuals/indivdiff.hpp"
#include "pools/individuals/indivbind.hpp"
#include "pools/mutations/mutactive.hpp"
#include "pools/mutations/mutpassive.hpp"
#include "pools/mutations/mutdiff.hpp"
#include "pools/mutations/mutbind.hpp"


template<typename T>
concept Pool = requires(typename T::Active a,
						typename T::Passive p,
						typename T::Diff d,
						Time t,
						PatchID pid,
						unsigned u,
						double dbl,
						RNGcore * rng){
	// Passive
	new T::Passive(pid);		new T::Passive(pid,u);
	p.size();
	d = p.generate(t, u);		d = p.generate(t, d);
	// Active
	new T::Active(pid, u, dbl);
	a.size();
	{a.getPhi()} -> std::same_as<double>;
	d = a.getNewErased(rng);		d = a.sampleInfectors(rng, u);
	a.shift(rng);
	// Diff
	new T::Diff(pid);
	d.size();
	d += d;
	d.moveFromTo(t,p,a);			d.moveFromTo(t,a,p);			d.moveFromTo(t,a,a);
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


struct Mutations{
	using Active = MutActive;
	using Passive = MutPassive;
	using Diff = MutDiff;
};

#endif