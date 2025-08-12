import sys
import os
from tabnanny import check
from tokenize import group
sys.path[0] = os.getcwd()

import h5py
import multiprocessing as mp
import numpy as np
import pandas as pd
from tqdm import tqdm
from argparse import ArgumentParser
from TravelAndMutate.argumenthelper import splitInput
from TravelAndMutate.datamanager import checkIsH5Group, checkIsH5Dataset
from TravelAndMutate.trees import Tree


def writeResult(outfilename, groupname, result):
	with h5py.File(outfilename, "a") as outfile:
		group = outfile.require_group(groupname)
		if "VirNA_accuracy" in group:
			del group["VirNA_accuracy"]
		group.create_dataset("VirNA_accuracy", data=result, compression="gzip", compression_opts=9)


def kernel(tpl):
	infilename, groupname, quiet = tpl
	with h5py.File(infilename) as infile:
		group = checkIsH5Group(infile[groupname])
		replica_paths = [replica.name for replica in group.values() if "VirNA_network" in replica]
	if not quiet:
		replica_paths = tqdm(replica_paths, desc=f"Processing replicas", miniters=1, mininterval=1, dynamic_ncols=True)

	data = []
	for replica_path in replica_paths:
		with h5py.File(infilename) as infile:
			mutationtree = checkIsH5Dataset(infile[replica_path+"/mutationtree"]).fields(["parent", "child"])[:]
			sampled_seqs = checkIsH5Dataset(infile[replica_path+"/sequencings"]).fields("id")[:]
			edgelist_virna = checkIsH5Dataset(infile[replica_path+"/VirNA_network"])[:]
			seed = checkIsH5Group(infile[replica_path]).attrs["seed"]
		try:
			tree_full = Tree(mutationtree)
			tree_actual = tree_full.subset(sampled_seqs)
			edgelist_true = tree_actual.getEdgelist()
			truePos = np.isin(edgelist_virna, edgelist_true).sum()
			falsePos = np.isin(edgelist_virna, edgelist_true, invert=True).sum()
			falseNeg = np.isin(edgelist_true, edgelist_virna, invert=True).sum()
			precision = truePos / (truePos + falsePos) if (truePos + falsePos) > 0 else 0
			recall = truePos / (truePos + falseNeg) if (truePos + falseNeg) > 0 else 0
			f1_score = 2 * (precision * recall) / (precision + recall) if (precision + recall) > 0 else 0
			data.append((seed, truePos, falsePos, falseNeg, precision, recall, f1_score))
		except Exception as exception:
			print(f"Error processing {seed}: {exception}")

	data = pd.DataFrame(data, columns=["seed", "truePos", "falsePos", "falseNeg", "precision", "recall", "f1_score"])
	return groupname, data.to_records(index=False)


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--name", type=str, required=True, help="Name of the input file (without extension)")
	parser.add_argument("--group", type=str, default="all", help="Name of the group to process. [default: all]")
	parser.add_argument("--nprocs", type=int, default=1, help="Number of processes to use. [default: 1]")
	parser.add_argument("--overwrite", type=bool, default=True, help="Overwrite existing output file. [default: True]")
	args = parser.parse_args()

	infilename = "data/" + args.name + ".h5"
	if not os.path.exists(infilename):
		print(f"Input file {infilename} does not exist.")
		sys.exit(1)
	if not infilename.endswith(".h5"):
		print(f"Input file {infilename} is not a HDF5 file.")
		sys.exit(1)
	outfilename = infilename.replace("data", "analysis")

	groupnames = args.group
	if groupnames == "all":
		with h5py.File(infilename) as infile:
			groupnames = list(infile.keys())
	else:
		groupnames = splitInput(groupnames)
	
	overwrite = args.overwrite
	if not overwrite and os.path.exists(outfilename):
		with h5py.File(outfilename) as outfile:
			to_remove = []
			for groupname in groupnames:
				if groupname in outfile:
					if "VirNA_accuracy" in checkIsH5Group(outfile[groupname]):
						to_remove.append(groupname)
		groupnames = [g for g in groupnames if g not in to_remove]

	nprocs = min(args.nprocs, len(groupnames))
	iterable = [(infilename, groupname, nprocs>1) for groupname in groupnames]
	if nprocs > 1:
		with mp.Pool(nprocs) as workers:
			results = workers.imap_unordered(kernel, iterable)
			for groupname, result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				writeResult(outfilename, groupname, result)
	else:
		for item in tqdm(iterable, desc="Processing groups", miniters=1, mininterval=1, dynamic_ncols=True):
			groupname, result = kernel(item)
			writeResult(outfilename, groupname, result)
