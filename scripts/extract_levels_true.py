import sys
import os
sys.path[0] = os.getcwd()

import h5py
import numpy as np
import pandas as pd
from tqdm import tqdm
import multiprocessing as mp
from argparse import ArgumentParser
from TravelAndMutate.datamanager import checkIsH5Dataset, checkIsH5Group
from TravelAndMutate.trees import Tree


def kernel(tpl):
	inputfilename = tpl[0]
	simname = f"{tpl[1]}/seed-{tpl[2]:05d}"
	with h5py.File(inputfilename) as infile:
		try:
			sim = checkIsH5Group(infile[simname])
			edgelist = checkIsH5Dataset(sim["mutationtree"]).fields(["parent","child"])[:]
			sequencings = checkIsH5Dataset(sim["sequencings"]).fields("id")[:]
		except Exception as exception:
			print(f"Error processing {simname}: {repr(exception)}")
			return pd.DataFrame(), True
	
	try:
		tree = Tree(edgelist)
		subtree = tree.subset(sequencings)
		subedgelist = subtree.getEdgelist()
		df = pd.DataFrame.from_records(subedgelist)
		df["levels"] = tree.computeDistanceFromAncestor(subedgelist)
		df.rename(columns={"parent": "source", "child": "target"}, inplace=True)
		return df, False
	except Exception as exception:
		print(f"Error processing {simname}: {repr(exception)}")
		return pd.DataFrame(), True


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	parser.add_argument("--nprocs", type=int, default=0)
	args = parser.parse_args()
	infilename = args.file
	if not infilename.endswith(".h5"):
		infilename += ".h5"
	if not os.path.exists(infilename):
		print(f"File {infilename} does not exist.")
		sys.exit(1)
	groupname = args.group

	with h5py.File(infilename, "r") as file:
		try:
			group = checkIsH5Group(file[groupname])
		except Exception as exception:
			print(f"Group {groupname} does not exist in file {infilename}.")
			file.close()
			sys.exit(1)
		iterable = [(infilename, groupname, sim.attrs["seed"]) for simname, sim in group.items() if sim.attrs["survived"]]

	nprocs = args.nprocs
	errors = []
	results = []
	if nprocs > 1:
		with mp.Pool(nprocs) as workers:
			outcome = workers.imap(kernel, iterable)
			for res in tqdm(outcome, total=len(iterable), desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
				results.append(res[0])
				errors.append(res[1])
	else:
		for tpl in tqdm(iterable, desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
			res = kernel(tpl)
			results.append(res[0])
			errors.append(res[1])
	print(f"Errors: {sum(errors)}")

	seeds = [tpl[2] for tpl in iterable]
	full_df = pd.concat(results, keys=seeds, names=["seed", "dummy"]).droplevel("dummy").reset_index().astype("u4")
	data = full_df.to_records(index=False)

	outfilename = infilename.replace("data/", "analysis/")
	with h5py.File(outfilename, "a") as outputfile:
		group = outputfile.require_group(groupname)
		if "levels_true" in group:
			del group["levels_true"]
		group.create_dataset("levels_true", data=data, compression="gzip", compression_opts=9)

	print(f"Results saved to {outfilename} in group {groupname}")