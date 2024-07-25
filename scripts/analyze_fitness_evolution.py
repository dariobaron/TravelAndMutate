import sys
import os
sys.path[0] = os.getcwd()

import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np
import h5py
import pandas as pd
from tqdm import tqdm
from TravelAndMutate.datamanager import checkIsH5Dataset, checkIsH5Group, filterGroupmembersWithParams
from TravelAndMutate.analyzer import writeDataset
from TravelAndMutate.argumenthelper import splitInput


def kernel(tpl):
	try:
		infilename = tpl[0]
		groupname = tpl[1]
		with h5py.File(infilename) as infile:
			group = checkIsH5Group(infile[groupname])
			dt = group.attrs["dt"]
			seedstolook = filterGroupmembersWithParams(group, {"survived":True})
		if seedstolook is None:
			raise RuntimeError(f"There are no valid seeds for this group")
		if not isinstance(seedstolook, list):
			seedstolook = [seedstolook]
		result = []
		for i in range(len(seedstolook)):
			with h5py.File(infilename) as infile:
				run = checkIsH5Group(infile[groupname+"/"+seedstolook[i]])
				seed = run.attrs["seed"]
				infections = checkIsH5Dataset(run["infections"]).fields(["t","mut"])[:]
				fitness = checkIsH5Dataset(run["fitness"])[:]
			fitness = pd.DataFrame.from_records(fitness, index="id")
			df = pd.DataFrame.from_records(infections, index="mut")
			df["phi"] = fitness.loc[df.index]
			df.set_index(np.full(df.shape[0], seed, dtype="u4"), inplace=True)
			df.index.name = "seed"
			result.append(df)
		result = pd.concat(result).to_records()
		return groupname,result	
	except Exception as exception:
		print(repr(exception))
		print(f"Occurred for group {groupname}")
		return groupname, None


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
		for groupname in tqdm(groupnames, miniters=1, mininterval=1, dynamic_ncols=True):
			_,result = kernel((infilename, groupname))
			if result is not None:
				writeDataset(outfilename, groupname, "fitness_evolution", attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				if result is not None:
					writeDataset(outfilename, groupname, "fitness_evolution", attributes[groupname], result)