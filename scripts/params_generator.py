import sys
import os
sys.path[0] = os.getcwd()

import json
import h5py
from argparse import ArgumentParser
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
	"mutation_rate" : 0.005,
	"mutation_k" : 10,
	"fitness_p" : 0.8,
	"fitness_delta-" : -0.1,
	"fitness_delta+" : +0.02,
	"sequencing_prob" : 0.00772,
	"reporting_prob" : 0.25,
	"sequencing_delay" : 20
}


def strToFloatOrInt(value):
	try:
		casted = int(value)
	except:
		try:
			casted = float(value)
		except:
			casted = value
	finally:
		return casted

parser = ArgumentParser(allow_abbrev=False)
parser.add_argument("--dir", type=str, required=True)
parser.add_argument("--name", type=str, required=True)
parser.add_argument("--force", type=str, default="", help="'--' separates different parameters, ':' separates key and value, '[' ',' ']' are used to assign a list to value. Example: N_patches:3--betas_params:[0.1,0.5,0.8]")
args = parser.parse_args()

forced_params = args.force
if len(forced_params) > 0:
	forced_params = forced_params.split("--")
	for p in forced_params:
		key,val = p.split(":")
		if key not in params_dict:
			raise RuntimeError(f"Impossible to force parameter {key}: it is not a parameter")
		if val[0] == "[" and val[-1]=="]":
			val = val[1:-1].split(",")
			for i,v in enumerate(val):
				val[i] = strToFloatOrInt(v)
		else:
			val = strToFloatOrInt(val)
		params_dict[key] = val

outputfolder = args.dir
if outputfolder[-1] != "/":
	outputfolder = outputfolder + "/"
if not os.path.isdir(outputfolder):
	os.mkdir(outputfolder)

name = args.name
filename = outputfolder + name

params = Params(params_dict)

try:
	datafile = h5py.File(filename+".h5", 'a')
	read_only = False
except:
	datafile = h5py.File(filename+".h5")
	read_only = True
finally:
	groupname = datman.filterGroupmembersWithParams(datafile, params.getGroupParams())
	if groupname is None:
		groupname = datman.createNewGroupname(datafile)
		if read_only:
			print(f"WARNING: {filename}.h5 cannot be modified. Consolidate these simulations before creating other parameter sets!")
		else:
			group = datafile.create_group(groupname, track_order=True)
			for key,val in params.getGroupParams().items():
				group.attrs.create(key, val)
	else:
		try:
			datman.checkAttributes(datafile[groupname], params.getGroupParams())
		except:
			datafile.close()
			raise
	datafile.close()

with open(f"{outputfolder+name}_{groupname}.json", "w") as paramsfile:
	json.dump(params_dict, paramsfile)
	paramsfile.write("\n")