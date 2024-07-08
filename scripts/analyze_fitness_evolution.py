import sys
import os
sys.path[0] = os.getcwd()

import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np
import h5py
import pandas as pd
from tqdm import tqdm
from TravelAndMutate.datamanager import checkAttributes, checkIsH5Dataset


def splitInput(string):
	names = []
	if "," in string:
		string = string.split(",")
	if not isinstance(string, list):
		string = [string]
	for el in string:
		if ":" in el:
			start,finish = el.split(":")
			names.extend(range(int(start),int(finish)))
		else:
			names.append(int(el))
	names = [str(name) for name in names]
	return names


def kernel(tpl):
	infilename = tpl[0]
	groupname = tpl[1]
	with h5py.File(infilename) as infile:
		group = infile.require_group(groupname)
		seedstolook = [label for label,run in group.items() if run.attrs["survived"]]
		allfitness = [checkIsH5Dataset(group[f"{name}/fitness"])[:] for name in seedstolook]
		allinfections = [checkIsH5Dataset(group[f"{name}/infections"]).fields(["t","mut"])[:] for name in seedstolook]
		for array in allinfections:
			array["t"] = np.round(array["t"] * group.attrs["dt"])
		result = []
		for i in range(len(seedstolook)):
			fitness = pd.DataFrame.from_records(allfitness[i], index="id")
			df = pd.DataFrame.from_records(allinfections[i], index="mut")
			df["phi"] = fitness.loc[df.index]
			df.reset_index(inplace=True, drop=True)
			result.append(df)
		result = pd.concat(result).to_records(index=False)
		return groupname,result


def writeToFile(outfilename, groupname, attributes, data):
	with h5py.File(outfilename, 'a') as outfile:
		if groupname in outfile:
			group = outfile.require_group(groupname)
			checkAttributes(group, attributes)
		else:
			group = outfile.create_group(groupname, track_order=True)
			for key,val in attributes.items():
				group.attrs[key] = val
		if "fitness_evolution" in group:
			del group["fitness_evolution"]
		dataset = group.create_dataset("fitness_evolution", data=data, compression="gzip", compression_opts=9)
	


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, default="")
	parser.add_argument("--nprocs", type=int, default=0)
	args = parser.parse_args()
	infilename = args.file
	if not os.path.isfile(infilename):
		raise RuntimeError(f"{infilename} is not a file")
	outfilename = infilename.replace("data/", "analysis/")
	with h5py.File(infilename) as infile:
		groupnames = args.group
		if groupnames == "":
			groupnames = list(infile.keys())
		else:
			groupnames = splitInput(groupnames)
		attributes = {groupname : dict(infile[groupname].attrs) for groupname in groupnames}
	nprocs = args.nprocs
	if nprocs < 2:
		for groupname in tqdm(groupnames):
			_,result = kernel((infilename, groupname))
			writeToFile(outfilename, groupname, attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable)):
				writeToFile(outfilename, groupname, attributes[groupname], result)