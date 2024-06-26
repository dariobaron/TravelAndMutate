from ast import arg
import sys
import os
sys.path[0] = os.getcwd()

from scripts.run_mutation import main as runner
import multiprocessing as mp
from argparse import ArgumentParser
import numpy as np


def splitInput(string):
	intlist = []
	if "--" in string:
		string = string.split("--")
	if not isinstance(string, list):
		string = [string]
	for el in string:
		if ":" in el:
			start,finish = el.split(":")
			intlist.extend(range(int(start),int(finish)))
		else:
			intlist.append(int(el))
	return intlist


def kernelNSucc(tpl):
	working_dir = tpl[0]
	filename = tpl[1]
	groupname = tpl[2]
	seed = tpl[3]
	stilltorun = tpl[4]
	while stilltorun > 0:
		ninfections = runner(working_dir, filename, groupname, seed, True)
		if ninfections > 100:
			stilltorun = stilltorun - 1
		seed = seed + 1
		if seed >= 1000:
			break
	return True


def kernelSeed(tpl):
	runner(tpl[0], tpl[1], tpl[2], tpl[3], True)
	return True


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	parser.add_argument("--seed", type=str, default="0")
	parser.add_argument("--nsucc", type=int, default=10)
	parser.add_argument("--nprocs", type=int, required=True)
	args = parser.parse_args()
	working_dir = args.dir
	filename = args.name
	groups = splitInput(args.group)
	seeds = splitInput(args.seed)
	nsucc = args.nsucc
	nprocs = args.nprocs
	if nsucc != -1 and len(seeds) != 1:
		print("nsucc cannot concile with the sequence of seeds provided")
		nsucc = -1
	with mp.Pool(nprocs) as workers:
		if nsucc == -1:
			iterable = [(working_dir, filename, group, seed) for group in groups for seed in seeds]
			results = workers.imap_unordered(kernelSeed, iterable)
		else:
			iterable = [(working_dir, filename, group, seeds[0], nsucc) for group in groups]
			results = workers.imap_unordered(kernelNSucc, iterable)
		for i in range(len(iterable)):
			element = next(results, False)
			print(f"Completed {(i+1)*100//len(iterable)}%", end="\r", flush=True)