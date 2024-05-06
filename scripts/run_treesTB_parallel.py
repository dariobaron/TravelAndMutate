import sys
import os
sys.path[0] = os.getcwd()

import time
import multiprocessing
from tqdm import tqdm
from argparse import ArgumentParser
from TravelAndMutate.datamanager import consolidateH5
from scripts import run_treesTB

parser = ArgumentParser(allow_abbrev=False)
parser.add_argument("--dir", type=str, required=True)
parser.add_argument("--seeds", type=str, required=True)
parser.add_argument("--nprocs", type=int, required=True)
args = parser.parse_args()
working_dir = args.dir
nprocs = args.nprocs
try:
	begin_idx, end_idx = args.seeds.split(":")
	seeds = [i for i in range(int(begin_idx), int(end_idx))]
except:
	raise ValueError("USAGE: --seeds parameter must be passed as BEGINIDX:ENDIDX (where ENDIDX is excluded)")

iterable = [seed for seed in seeds]
def kernel(seed):
	run_treesTB.main(working_dir, "treesTB", seed, suppress_output=True)
	return

if __name__ == '__main__' and kernel is not None:
	mp_context = multiprocessing.get_context("forkserver")
	with mp_context.Pool(nprocs) as pool:
		chunksize = max(len(iterable)//nprocs//10, 1)
		results = pool.imap_unordered(kernel, iterable, chunksize=chunksize)
		with tqdm(iterable=results, desc="Progress", total=len(iterable), ncols=80, leave=False) as pbar:
			for result in results:
				pbar.update(1)
		pool.close()
		pool.join()
	filename = "data/treesTB"
	consolidateH5(filename, filename)

