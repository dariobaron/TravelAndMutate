import sys
import os
sys.path[0] = os.getcwd()

import json
import h5py
from argparse import ArgumentParser
import itertools
import multiprocessing as mp
from tqdm import tqdm, trange
from TravelAndMutate.paramsmanager import Params
import TravelAndMutate.datamanager as datman
from scripts.params_generator import params_dict


def kernel(tpl):
	filename, params = tpl
	with h5py.File(filename) as datafile:
		groupname = datman.filterGroupmembersWithParams(datafile, params.getGroupParams())
		if groupname is not None:
			try:
				datman.checkAttributes(datafile[groupname], params.getGroupParams())
			except:
				groupname = "WRONG"
	return groupname,True


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--spanning", type=str, required=True, help="json file to read spanning parameters")
	parser.add_argument("--nprocs", type=int, default=0)
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
	filename = outputfolder + name + ".h5"

	updaters = [dict(zip(multiple_params.keys(),values_set)) for values_set in itertools.product(*multiple_params.values())]
	params_dicts = [params_dict.copy() for i in updaters]
	for origdicts,updater in zip(params_dicts,updaters):
		origdicts.update(updater)
	allparams = [Params(params_dict) for params_dict in params_dicts]

	datafile = h5py.File(filename, "a")
	nprocs = args.nprocs
	if nprocs > 1:
		datafile.close()
		with mp.Pool(nprocs) as workers:
			results = workers.imap(kernel, [(filename,params) for params in allparams], chunksize=max(1,min(10,len(allparams)//(10*nprocs))))
			groupnames = []
			for groupname,_ in tqdm(results, total=len(allparams), desc="Setting groupnames", miniters=1, mininterval=1, dynamic_ncols=True):
				groupnames.append(groupname)
			groupnames = list(groupnames)
		datafile = h5py.File(filename, "a")
	else:
		groupnames = [kernel((filename,params))[0] for params in allparams]
	for i in trange(len(allparams), desc="Creating new groups", miniters=1, mininterval=1, dynamic_ncols=True):
		if groupnames[i] is None:
			groupnames[i] = datman.createNewGroupname(datafile)
			group = datafile.create_group(groupnames[i], track_order=True)
			for key,val in allparams[i].getGroupParams().items():
				group.attrs.create(key, val)
		elif groupnames[i] == "WRONG":
			datafile.close()
			raise RuntimeError("Attributes were not matched correctly")
	datafile.close()

	for params_dict,groupname in tqdm(zip(params_dicts,groupnames), desc="Writing jsons", miniters=1, mininterval=1, dynamic_ncols=True):
		with open(f"{outputfolder+name}_{groupname}.json", "w") as paramsfile:
			json.dump(params_dict, paramsfile)
			paramsfile.write("\n")