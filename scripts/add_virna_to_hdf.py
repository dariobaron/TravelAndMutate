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
from TravelAndMutate.datamanager import checkIsH5Group


def kernel(inputfilename):
	try:
		graph = nx.read_gml(inputfilename)
		edgelist = []
		for a in nx.to_edgelist(graph):
			edgelist.append([a[0], a[1]])
		edgelist = pd.DataFrame(edgelist, columns=["parent","child"]).astype("u4")
		edgelist.sort_values(by=["parent", "child"], inplace=True)
		edgelist = edgelist.to_records(index=False)
		return edgelist
	except Exception as exception:
		print(f"Occurred for file {inputfilename}", end="\t:\t")
		print(repr(exception))
		return np.empty(0, dtype=[("parent", "u4"), ("child", "u4")])


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

	seeds = [int(filename.replace(basename, "").replace(postfix, "")) for filename in filenames]
	
	outfilename = args.outputfile
	if not outfilename.endswith(".h5"):
		outfilename += ".h5"
	with h5py.File(outfilename, "a") as outputfile:
		group = outputfile.require_group(groupname)

		nprocs = args.nprocs
		i = 0
		if nprocs > 1:
			with mp.Pool(nprocs) as workers:
				results = workers.imap(kernel, filenames)
				for result in tqdm(results, total=len(filenames), desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
					if result.shape[0] == 0:
						print(f"Empty result for {filenames[i]}, skipping.")
						continue
					sim = checkIsH5Group(group[f"seed-{seeds[i]:05d}"])
					if "VirNA_network" in sim.keys():
						del sim["VirNA_network"]
					sim.create_dataset("VirNA_network", data=result, compression="gzip", compression_opts=9)
					i += 1
		else:
			for filename in tqdm(filenames, desc="Processing network", miniters=1, mininterval=1, dynamic_ncols=True):
				result = kernel(filename)
				if result.shape[0] == 0:
					print(f"Empty result for {filename}, skipping.")
					continue
				sim = checkIsH5Group(group[f"seed-{seeds[i]:05d}"])
				if "VirNA_network" in sim:
					del sim["VirNA_network"]
				sim.create_dataset("VirNA_network", data=result, compression="gzip", compression_opts=9)
				i += 1

	print(f"Results saved to {outfilename} in group {groupname}")