#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "types.hpp"

struct Individual{
	unsigned id_, infector_id_;
	PatchID patch_, infector_patch_;
	Individual() : id_(-1), infector_id_(-1), patch_(-1), infector_patch_(-1) {}
	Individual(PatchID patch, unsigned id, PatchID inf_patch=-1, unsigned inf_id=-1) :
		id_(id), infector_id_(inf_id), patch_(patch), infector_patch_(inf_patch) {}
};

#endif