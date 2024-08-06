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
from scipy.signal import find_peaks
from TravelAndMutate.datamanager import checkIsH5Dataset, checkIsH5Group, filterGroupmembersWithParams, collectAttributeFromGroup
from TravelAndMutate.analyzer import writeDataset, writeEmpty
from TravelAndMutate.argumenthelper import splitInput


def kernel(tpl):
	try:
		infilename = tpl[0]
		groupname = tpl[1]
		only_survived = tpl[2]
		with h5py.File(infilename) as infile:
			group = checkIsH5Group(infile[groupname])
			if only_survived:
				seedstolook = filterGroupmembersWithParams(group, {"survived":True})
			else:
				seedstolook = list(group.keys())
			nruns_attempted = len(group.keys())
		if seedstolook is None:
			raise RuntimeError(f"There are no valid seeds for this group")
		if not isinstance(seedstolook, list):
			seedstolook = [seedstolook]
		result = []
		for i in range(len(seedstolook)):
			with h5py.File(infilename) as infile:
				run = checkIsH5Group(infile[groupname+"/"+seedstolook[i]])
				seed = run.attrs["seed"]
				trajectories = {int(loc_id) : trajectory.fields(["t","I"])[:] for loc_id,trajectory in checkIsH5Group(run["trajectories"]).items()}
			for loc_id,trajectory in trajectories.items():
				if np.sum(trajectory["I"]>0) != 0:
					t_invasion = trajectory["t"][trajectory["I"]>0][0]
					t_extinction = trajectory["t"][trajectory["I"]>0][-1]
				else:
					t_invasion = -1
					t_extinction = -1
				result.append([seed, loc_id, 0, t_invasion])
				result.append([seed, loc_id, -1, t_extinction])
				peaks = find_peaks(trajectory["I"], width=30, distance=60, prominence=10)[0]
				for j,peak_idx in enumerate(peaks):
					t_peak = trajectory["t"][peak_idx]
					result.append([seed, loc_id, j+1, t_peak])
		result = pd.DataFrame(result, columns=["seed","loc_id","event","t"], dtype="i4")
		result["t"] = result["t"].astype("f4").replace(-1, np.nan)
		return groupname, result.to_records(index=False)
	except Exception as exception:
		print(f"Occurred for group {groupname}", end="\t:\t")
		print(repr(exception))
		return groupname, repr(exception)


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, default="")
	parser.add_argument("--nprocs", type=int, default=0)
	parser.add_argument("--overwrite", type=bool, default=False)
	parser.add_argument("--onlysurvived", type=bool, default=True)
	args = parser.parse_args()
	overwrite = args.overwrite
	only_survived = args.onlysurvived
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
					if "geography" in checkIsH5Group(outfile[groupname]):
						to_remove.append(groupname)
		groupnames = [groupname for groupname in groupnames if groupname not in to_remove]
	nprocs = args.nprocs
	if nprocs < 2:
		for groupname in tqdm(groupnames, miniters=1, mininterval=1, dynamic_ncols=True):
			_,result = kernel((infilename, groupname,only_survived))
			if isinstance(result, str):
				writeEmpty(outfilename, groupname, "geography", attributes[groupname], result)
			else:
				writeDataset(outfilename, groupname, "geography", attributes[groupname], result)
	else:
		with mp.Pool(nprocs-1) as workers:
			iterable = [(infilename,groupname,only_survived) for groupname in groupnames]
			results = workers.imap_unordered(kernel, iterable)
			for groupname,result in tqdm(results, total=len(iterable), miniters=1, mininterval=1, dynamic_ncols=True):
				if isinstance(result, str):
					writeEmpty(outfilename, groupname, "geography", attributes[groupname], result)
				else:
					writeDataset(outfilename, groupname, "geography", attributes[groupname], result)
