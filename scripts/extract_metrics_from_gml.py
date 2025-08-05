from operator import index
import sys
import os
sys.path[0] = os.getcwd()

import h5py
import numpy as np
import pandas as pd
from tqdm import tqdm
import multiprocessing as mp
from argparse import ArgumentParser
import networkx as nx
from glob import glob


def kernel(inputfilename):
	try:
		graph = nx.read_gml(inputfilename)
		df = pd.concat({
			"outDeg" : pd.Series(dict(graph.out_degree())),
			"inDeg" : pd.Series(dict(graph.in_degree())),
		}, axis=1)
		isolated_nodes = df.loc[np.logical_and(df["outDeg"]==0, df["inDeg"]==0)].index.tolist()
		df.drop(index=isolated_nodes, inplace=True)
		roots = df.loc[np.logical_and(df["outDeg"]!=0, df["inDeg"]==0)].index.tolist()
		depths = pd.concat([pd.Series(nx.shortest_path_length(graph, root)) for root in roots], axis=1)
		depths.drop(index=isolated_nodes, inplace=True, errors="ignore")
		df["depth"] = depths.min(axis=1).astype(int)
		df.index = df.index.astype(int)
		return df
	except Exception as exception:
		print(f"Occurred for file {inputfilename}", end="\t:\t")
		print(repr(exception))
		return pd.DataFrame()


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--inputfolder", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	parser.add_argument("--outputfile", type=str, required=True)
	parser.add_argument("--nprocs", type=int, default=0)
	args = parser.parse_args()
	inputfolder = args.inputfolder
	if not inputfolder.endswith("/"):
		inputfolder += "/"
	if not os.path.exists(inputfolder):
		print(f"Input folder {inputfolder} does not exist.")
		sys.exit(1)
	groupname = args.group
	basename = inputfolder + args.name + "_" + groupname + "_seed-"
	postfix = "-msn.gml"
	filenames = glob(basename + "*" + postfix)
	if len(filenames) == 0:
		print(f"No files found matching {basename}*-msn.gml")
		sys.exit(1)

	nprocs = args.nprocs
	if nprocs > 1:
		with mp.Pool(nprocs) as workers:
			results = workers.imap(kernel, filenames)
			done = []
			for res in tqdm(results, total=len(filenames), desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
				done.append(True)
	else:
		results = []
		for filename in tqdm(filenames, desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
			results.append(kernel(filename))

	seeds = [int(filename.replace(basename, "").replace(postfix, "")) for filename in filenames]
	full_df = pd.concat(results, keys=seeds, names=["seed", "node"]).reset_index().astype("u4")
	data = full_df.to_records(index=False)

	outfilename = args.outputfile
	if not outfilename.endswith(".h5"):
		outfilename += ".h5"
	with h5py.File(outfilename, "a") as outputfile:
		group = outputfile.require_group(groupname)
		if "metrics_virna_network" in group:
			del group["metrics_virna_network"]
		group.create_dataset("metrics_virna_network", data=data, compression="gzip", compression_opts=9)
	
	print(f"Results saved to {outfilename} in group {groupname}")