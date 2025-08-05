from operator import index
import sys
import os
sys.path[0] = os.getcwd()

import h5py
import numpy as np
from tqdm import tqdm
from argparse import ArgumentParser
from TravelAndMutate.datamanager import checkIsH5Group


def condition(simulation):
	threshold_samples = 150
	threshold_haplos = 30
	enough_haploss = np.unique(simulation["sequencings"].fields("id")[:]).shape[0] >= threshold_haplos
	enough_samples = simulation["sequencings"].shape[0] >= threshold_samples
	return enough_haploss and enough_samples


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--file", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	args = parser.parse_args()
	
	with h5py.File(args.file, "a") as file:
		if args.group not in file:
			print(f"Group {args.group} not found in {args.file}.")
			exit(1)
		
		try:
			group = checkIsH5Group(file[args.group])
		except ValueError as e:
			print(f"Error: {e}")
			exit(1)
		
		for key,simulation in tqdm(group.items(), miniters=1, mininterval=1, dynamic_ncols=True):
			try:
				survived = condition(simulation)
			except Exception as e:
				print(f"Error processing simulation {key}: {e}")
				continue
			if survived != simulation.attrs["survived"]:
				print(f"Updating simulation {key} survived status from {simulation.attrs['survived']} to {survived}.")
			simulation.attrs["survived"] = survived
		
		print(f"Processed {len(group)} simulations in group {args.group}.")
