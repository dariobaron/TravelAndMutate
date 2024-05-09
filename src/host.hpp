#ifndef HOST_HPP
#define HOST_HPP

#include "types.hpp"
#include "randomcore.hpp"

struct Host{
	Time t_last_mut_, t_next_mut_;
	unsigned id_, infector_id_;
	unsigned mut_, infector_mut_, evolved_mut_;
	PatchID patch_, infector_patch_;
	unsigned nmut_;
	
	Host() : Host(-1, -1, -1, -1, -1, -1, -1, -1) {};
	Host(Time t, Time tnext, PatchID patch, unsigned id, unsigned mut) : Host(t, tnext, patch, id, mut, -1, -1, -1) {};
	Host(Time t, Time tnext, PatchID patch, unsigned id, unsigned mut, PatchID inf_patch, unsigned inf_id, unsigned inf_mut) :
		t_last_mut_(t), t_next_mut_(tnext), id_(id), infector_id_(inf_id), mut_(mut), infector_mut_(inf_mut), evolved_mut_(mut), patch_(patch), infector_patch_(inf_patch), nmut_(0) {};

	void evolveMutation(Time t, unsigned new_mut, Time tnext){
		t_last_mut_ = t;
		mut_ = evolved_mut_;
		evolved_mut_ = new_mut;
		t_next_mut_ = tnext;
		++nmut_;
	};
};


#endif