#ifndef EVENTRECORDER_HPP
#define EVENTRECORDER_HPP

#include <vector>
#include "types.hpp"

struct StateTransition{
	Time t;
	unsigned loc;
	unsigned ID;
	unsigned newstate;
	StateTransition(Time t, unsigned loc, unsigned ID, char newstate) : t(t), loc(loc), ID(ID), newstate(newstate) {};
};

class EventRecorder{
private:
	Vec<StateTransition> trans_;
public:
	template<typename ...Args>
	void pushTransition(Args... args){
		trans_.emplace_back(args...);
	}
	np_array<StateTransition> read() const{
		np_array<StateTransition> toreturn(trans_.size());
		auto setter = toreturn.mutable_unchecked<1>();
		for (unsigned i = 0; i < trans_.size(); ++i){
			setter[i] = trans_[i];
		}
		return toreturn;
	};
};

static EventRecorder event_recorder;

#endif