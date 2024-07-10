import sys
import os
sys.path[0] = os.getcwd()

import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np
import h5py
import pandas as pd
from tqdm import tqdm
from TravelAndMutate.datamanager import checkIsH5Dataset, filterGroupmembersWithParams
from TravelAndMutate.analyzer import writeDataset
from TravelAndMutate.argumenthelper import splitInput
from TravelAndMutate.trees import Tree


def kernel(tpl):
	infilename = tpl[0]
	groupname = tpl[1]
	with h5py.File(infilename) as infile:
		group = infile.require_group(groupname)
		dt = group.attrs["dt"]
		seedstolook = filterGroupmembersWithParams(group, {"survived":True})
	if seedstolook is None:
		return groupname,None
	result = {}
	for i in range(len(seedstolook)):
		with h5py.File(infilename) as infile:
			run = infile.require_group(groupname+"/"+seedstolook[i])
			seed = run.attrs["seed"]
			infections = checkIsH5Dataset(run["infections"]).fields(["t","mut"])[:]
			mutationtree = checkIsH5Dataset(run["mutationtree"])[:]
		metrics = {}
		infectious_haplo = pd.DataFrame.from_records(infections).groupby("mut").count()
		metrics["InfByHaplos_max"] = (infectious_haplo.max() / infectious_haplo.sum())["t"]
		metrics["InfByHaplos_mean"] = (infectious_haplo.mean() / infectious_haplo.sum())["t"]
		tree = Tree(mutationtree[1:])
		depths = tree.computeDepths()
		metrics["TreeDepth_max"] = depths.max()
		metrics["TreeDepth_mean"] = depths.mean()
		children = tree.computeNChildrenPerNode()
		metrics["nChildren_max"] = children.max() / children.sum()
		metrics["nChildren_mean"] = children.mean() / children.sum()
		metrics["B2"] = tree.computeB2()
		metrics["B2Norm"] = tree.computeB2Norm()
		#metrics["Cophenetic"] = tree.computeCophenetic()
		#metrics["CopheneticNorm"] = tree.computeCopheneticNorm()
		result[seed] = metrics
	result = pd.DataFrame.from_dict(result, orient="index")
	result.index.name = "seed"
	return groupname, result.to_records()


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
			writeDataset(outfilename, groupname, "single_quantities", attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable)):
				writeDataset(outfilename, groupname, "single_quantities", attributes[groupname], result)