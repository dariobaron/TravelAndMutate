import sys
import os
sys.path[0] = os.getcwd()

import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np
import h5py
import pandas as pd
from datetime import datetime, timedelta
from TravelAndMutate.datamanager import checkIsH5Group


def appendSlash(string):
	if not string[-1] == "/":
		string = string + "/"
	return string


def writeFasta(simulation, outputfilename, patches):
	start_date = datetime(2020, 5, 2).date()
	try:
		sequencings = pd.DataFrame.from_records(simulation["sequencings"][:])
		sequences = pd.DataFrame.from_records(simulation["sequences"][:], index="id")["sequence"].str.decode("UTF-8")
	except Exception as exception:
		print(f"Error reading datasets from simulation {simulation.name}: {exception}")
		return False
	with open(outputfilename, "w") as fasta_file:
		for _,(t,loc,id) in sequencings.iterrows():
			date = start_date + timedelta(t)
			print(f">{id}", patches.loc[loc], date, sep="|", file=fasta_file)
			seq = sequences.loc[id]
			for chkpnt in range(int(np.ceil(len(seq)/80))):
				print(seq[chkpnt*80:(chkpnt+1)*80], file=fasta_file)
			print(file=fasta_file)
	return True


def main(input_dir, output_dir, basename, groupname, seed):
	if not os.path.exists(output_dir):
		print(f"Creating output directory at {output_dir}")
		os.makedirs(output_dir)
	inputfilename = f"{input_dir}{basename}.h5"
	try:
		file = h5py.File(inputfilename)
	except Exception as exception:
		print(f"Error opening file {inputfilename}: {exception}")
		return False
	simname = f"{groupname}/seed-{seed:05d}"
	try:
		group = checkIsH5Group(file[simname])
	except Exception as exception:
		print(f"Error accessing group {simname}: {exception}")
		file.close()
		return False
	outputfilename = f"{output_dir}{basename}_{groupname}_seed-{seed:05d}.fasta"
	patchID = pd.read_csv("inputparams/patchIDs/england.csv", index_col="patchID")["name"]
	patchID = patchID.str.replace(" ", "_")
	try:
		writeFasta(group, outputfilename, patchID)
	except Exception as exception:
		print(f"Error writing FASTA file: {exception}")
		file.close()
		return False
	file.close()
	return True


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	parser.add_argument("--seed", type=int, required=True)
	parser.add_argument("--outdir", type=str, default="")
	args = parser.parse_args()
	input_dir = appendSlash(args.dir)
	basename = args.name
	group = args.group
	seed = args.seed
	if args.outdir == "":
		output_dir = input_dir
	else:
		output_dir = appendSlash(args.outdir)
	main(input_dir, output_dir, basename, group, seed)