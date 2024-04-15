#ifndef HOST_HPP
#define HOST_HPP

#include "types.hpp"
#include "randomcore.hpp"

struct Host{
	Time t_last_mut_, t_next_mut_;
	unsigned id_, infector_id_;
	unsigned mut_, infector_mut_, evolved_mut_;
	PatchID patch_, infector_patch_;
	
	Host() : Host(-1, -1, -1, -1, -1) {};
	Host(Time t, Time tnext, PatchID patch, unsigned id, unsigned mut, PatchID inf_patch=-1, unsigned inf_id=-1, unsigned inf_mut=-1) :
		t_last_mut_(t), t_next_mut_(tnext), id_(id), infector_id_(inf_id), mut_(mut), infector_mut_(inf_mut), evolved_mut_(mut), patch_(patch), infector_patch_(inf_patch) {};
	
/*	bool operator==(const Host & other){
		return patch_ == other.patch_ && id_ == other.id_ && mut_ == other.mut_;
	};
*/	
	void evolveMutation(Time t, unsigned new_mut, Time tnext){
		t_last_mut_ = t;
		mut_ = evolved_mut_;
		evolved_mut_ = new_mut;
		t_next_mut_ = tnext;
	};
};


#endif