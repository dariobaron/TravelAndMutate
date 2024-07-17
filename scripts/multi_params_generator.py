import sys
import os
sys.path[0] = os.getcwd()

import json
import h5py
from argparse import ArgumentParser
import itertools
from TravelAndMutate.paramsmanager import Params
import TravelAndMutate.datamanager as datman

params_dict = {
	"sys_type" : "mutations",
	"N_patches" : 107,
	"dt" : 0.1,
	"t_max" : 10000,
	"gamma_trick" : 3,
	"Ns_setter" : "fromcsv",
	"Ns_params" : "inputparams/Ns/italy_small.csv",
	"commuting_setter" : "fromcsv",
	"commuting_params" : "inputparams/c_ij/italy_small.csv",
	"betas_setter" : "delta",
	"betas_params" : 0.03,
	"epsilons_setter" : "delta",
	"epsilons_params" : 0.1,
	"mus_setter" : "delta",
	"mus_params" : 0.02,
	"I0_setter" : "onehot",
	"I0_params" : [27,1],
	"mutation_rate" : 0.008,
	"mutation_k" : 10,
	"fitness_p" : 0.8,
	"fitness_delta-" : -0.1,
	"fitness_delta+" : 0.02,
	"sequencing_prob" : 0.00772,
	"reporting_prob" : 0.25,
	"sequencing_delay" : 20
}


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--spanning", type=str, required=True, help="json file to read spanning parameters")
	args = parser.parse_args()

	outputfolder = args.dir
	if outputfolder[-1] != "/":
		outputfolder = outputfolder + "/"
	if not os.path.isdir(outputfolder):
		os.mkdir(outputfolder)

	spanningfilename = args.spanning
	if not os.path.isfile(spanningfilename):
		raise RuntimeError("Spanning file does not exists")
	
	with open(spanningfilename) as spanningfile:
		multiple_params = json.load(spanningfile)

	name = args.name
	filename = outputfolder + name

	updaters = [dict(zip(multiple_params.keys(),values_set)) for values_set in itertools.product(*multiple_params.values())]
	params_dicts = [params_dict.copy() for i in updaters]
	for origdicts,updater in zip(params_dicts,updaters):
		origdicts.update(updater)
	allparams = [Params(params_dict) for params_dict in params_dicts]

	datafile = h5py.File(filename+".h5", 'a')
	groupnames = [datman.filterGroupmembersWithParams(datafile, params.getGroupParams()) for params in allparams]
	for i in range(len(allparams)):
		if groupnames[i] is None:
			groupnames[i] = datman.createNewGroupname(datafile)
			group = datafile.create_group(groupnames[i], track_order=True)
			for key,val in allparams[i].getGroupParams().items():
				group.attrs.create(key, val)
		else:
			try:
				datman.checkAttributes(datafile[groupnames[i]], allparams[i].getGroupParams())
			except:
				datafile.close()
				raise
	datafile.close()

	for params_dict,groupname in zip(params_dicts,groupnames):
		with open(f"{outputfolder+name}_{groupname}.json", "w") as paramsfile:
			json.dump(params_dict, paramsfile)
			paramsfile.write("\n")