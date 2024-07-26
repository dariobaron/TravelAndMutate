from genericpath import isfile
import sys
import os
sys.path[0] = os.getcwd()

import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np
import h5py
import pandas as pd
from tqdm import tqdm
from TravelAndMutate.datamanager import checkIsH5Dataset, checkIsH5Group, filterGroupmembersWithParams, collectAttributeFromGroup
from TravelAndMutate.analyzer import writeDataset
from TravelAndMutate.argumenthelper import splitInput
from TravelAndMutate.trees import Tree


def kernel(tpl):
	try:
		infilename = tpl[0]
		groupname = tpl[1]
		with h5py.File(infilename) as infile:
			group = checkIsH5Group(infile[groupname])
			seedstolook = filterGroupmembersWithParams(group, {"survived":True})
			nruns_attempted = len(group.keys())
		if seedstolook is None:
			raise RuntimeError(f"There are no valid seeds for this group")
		if not isinstance(seedstolook, list):
			seedstolook = [seedstolook]
		result = {}
		for i in range(len(seedstolook)):
			with h5py.File(infilename) as infile:
				run = checkIsH5Group(infile[groupname+"/"+seedstolook[i]])
				seed = run.attrs["seed"]
				infections = checkIsH5Dataset(run["infections"]).fields(["t","mut"])[:]
				mutationtree = checkIsH5Dataset(run["mutationtree"])[:]
				sequencings = checkIsH5Dataset(run["sequencings"])[:]
			metrics = {}
			survival_rate = len(seedstolook) / nruns_attempted
			metrics["survivalrate"] = survival_rate
			infectious_haplo = pd.DataFrame.from_records(infections).groupby("mut").count()
			metrics["InfByHaplos_max"] = (infectious_haplo.max() / infectious_haplo.sum())["t"]
			metrics["InfByHaplos_mean"] = (infectious_haplo.mean() / infectious_haplo.sum())["t"]
			tree = Tree(mutationtree)
			depths = tree.computeDepths()
			metrics["TreeDepth_max"] = depths.max()
			metrics["TreeDepth_mean"] = depths.mean()
			children = tree.computeNChildrenPerNode()
			metrics["nChildren_max"] = children.max() / children.sum()
			metrics["nChildren_mean"] = children.mean() / children.sum()
			metrics["B2"] = tree.computeB2()
			metrics["B2Norm"] = tree.computeB2Norm()
			metrics["Cophenetic"] = tree.computeCophenetic()
			metrics["CopheneticNorm"] = tree.computeCopheneticNorm()
			sequenced_ids,counts = np.unique(sequencings["id"], return_counts=True)
			metrics["SequencingsByHaplos_max"] = (counts.max() / counts.sum())
			metrics["SequencingsByHaplos_2ndmax"] = (counts.max() / np.sort(counts)[-2])
			subtree = tree.subset(sequencings["id"])
			subdepths = subtree.computeDepths()
			metrics["SubTreeDepth_max"] = subdepths.max()
			metrics["SubTreeDepth_mean"] = subdepths.mean()
			subchildren = subtree.computeNChildrenPerNode()
			metrics["SubNChildren_max"] = subchildren.max() / subchildren.sum()
			metrics["SubNChildren_mean"] = subchildren.mean() / subchildren.sum()
			result[seed] = metrics
		result = pd.DataFrame.from_dict(result, orient="index")
		result.index.name = "seed"
		return groupname, result.to_records()
	except Exception as exception:
		print(f"Occurred for group {groupname}", end="\t:\t")
		print(repr(exception))
		return groupname, None


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
					if "single_quantities" in checkIsH5Group(outfile[groupname]):
						to_remove.append(groupname)
		groupnames = [groupname for groupname in groupnames if groupname not in to_remove]
	nprocs = args.nprocs
	if nprocs < 2:
		for groupname in tqdm(groupnames, miniters=1, mininterval=1, dynamic_ncols=True):
			_,result = kernel((infilename, groupname))
			if result is not None:
				writeDataset(outfilename, groupname, "single_quantities", attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				if result is not None:
					writeDataset(outfilename, groupname, "single_quantities", attributes[groupname], result)