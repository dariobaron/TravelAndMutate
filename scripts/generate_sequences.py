import sys
import os
from xml.sax.xmlreader import InputSource

from TravelAndMutate.analyzer import writeDataset
sys.path[0] = os.getcwd()

import h5py
import numpy as np
from tqdm import tqdm
import multiprocessing as mp
from argparse import ArgumentParser
from TravelAndMutate.argumenthelper import splitInput
from TravelAndMutate.datamanager import checkIsH5Dataset, checkIsH5Group
from TravelAndMutate.haplotypes import Haplotype


def kernel(tpl):
	try:
		filename = tpl[0]
		fullsimname = tpl[1]
		with h5py.File(filename) as file:
			mutationtree = checkIsH5Dataset(file[f"{fullsimname}/mutationtree"])[:]
			sampled_seqs = checkIsH5Dataset(file[f"{fullsimname}/sequencings"])[:]
		haplodealer = Haplotype(mutationtree)
		seqs_to_compute = np.unique(sampled_seqs["id"])
		sequences = haplodealer.read(seqs_to_compute)
		return fullsimname, sequences
	except Exception as exception:
		print(f"Occurred for group {fullsimname}", end="\t:\t")
		print(repr(exception))
		return fullsimname, np.empty(1)


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, default="")
	parser.add_argument("--seed", type=str, default="")
	parser.add_argument("--onlysurvived", type=bool, default=True)
	parser.add_argument("--overwrite", type=bool, default=False)
	parser.add_argument("--nprocs", type=int, default=0)
	args = parser.parse_args()
	filename = args.file
	with h5py.File(filename) as file:
		groupnames = args.group
		if groupnames == "":
			groupnames = list(file.keys())
		else:
			groupnames = splitInput(groupnames)
		seeds = args.seed
		if seeds == "":
			all_seeds = {name : list(group.keys()) for name,group in file.items()}
		else:
			input_seeds = splitInput(seeds)
			all_seeds = {groupname : [f"seed-{int(seed):05d}" for seed in input_seeds if f"seed-{int(seed):05d}" in checkIsH5Group(file[groupname]).keys()] for groupname in groupnames}
		onlysurvived = args.onlysurvived
		if onlysurvived:
			all_seeds = {groupname : [simname for simname in seeds if file[f"{groupname}/{simname}"].attrs["survived"]] for groupname,seeds in all_seeds.items()}
		overwrite = args.overwrite
		if not overwrite:
			all_seeds = {groupname : [simname for simname in seeds if "sequences" not in checkIsH5Group(file[f"{groupname}/{simname}"]).keys()] for groupname,seeds in all_seeds.items()}
	iterable = [(filename, f"{groupname}/{simname}") for groupname,seeds in all_seeds.items() for simname in seeds]
	nprocs = args.nprocs
	if nprocs < 2:
		results = []
		for iteration in tqdm(iterable, miniters=1, mininterval=1, dynamic_ncols=True):
			results.append(kernel(iteration))
	else:
		with mp.Pool(nprocs-1) as workers:
			results = workers.imap_unordered(kernel, iterable)
			for fullsimname,result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				_ = result.shape
	with h5py.File(filename, "a") as file:
		for fullsimname,result in results:
			sim = checkIsH5Group(file[fullsimname])
			if "sequences" in sim.keys():
				del sim["sequences"]
			sim.create_dataset("sequences", data=result)
