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
from TravelAndMutate.analyzer import writeDataset, writeEmpty
from TravelAndMutate.argumenthelper import splitInput
from TravelAndMutate.trees import Tree


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
				mutationtree = checkIsH5Dataset(run["mutationtree"]).fields(["parent","child"])[:]
			df = pd.DataFrame.from_records(infections, index="mut")
			tree = Tree(mutationtree)
			depths = tree.computeDepths()
			df["Nmutations"] = depths[df.index]
			df.set_index(np.full(df.shape[0], seed, dtype="u4"), inplace=True)
			df.index.name = "seed"
			result.append(df)
		result = pd.concat(result).to_records()
		return groupname,result
	except Exception as exception:
		print(repr(exception))
		print(f"Occurred for group {groupname}")
		return groupname, repr(exception)


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, default="")
	parser.add_argument("--nprocs", type=int, default=0)
	parser.add_argument("--overwrite", type=bool, default=False)
	args = parser.parse_args()
	overwrite = args.overwrite
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
	if os.path.isfile(outfilename) and not overwrite:
		to_remove = []
		with h5py.File(outfilename) as outfile:
			for groupname in groupnames:
				if groupname in outfile:
					if "Nmutations_evolution" in checkIsH5Group(outfile[groupname]):
						to_remove.append(groupname)
		groupnames = [groupname for groupname in groupnames if groupname not in to_remove]
	nprocs = args.nprocs
	if nprocs < 2:
		for groupname in tqdm(groupnames, miniters=1, mininterval=1, dynamic_ncols=True):
			_,result = kernel((infilename, groupname))
			if isinstance(result, str):
				writeEmpty(outfilename, groupname, "Nmutations_evolution", attributes[groupname], result)
			else:
				writeDataset(outfilename, groupname, "Nmutations_evolution", attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				if isinstance(result, str):
					writeEmpty(outfilename, groupname, "Nmutations_evolution", attributes[groupname], result)
				else:
					writeDataset(outfilename, groupname, "Nmutations_evolution", attributes[groupname], result)