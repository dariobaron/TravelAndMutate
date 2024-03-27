import sys
import os
sys.path[0] = os.getcwd()

import json
import time
import pandas as pd
import numpy as np
from argparse import ArgumentParser
from TravelAndMutate.paramsmanager import Params
from TravelAndMutate.randominterface import NumpyRandomGenerator
from TravelAndMutate.system import SystemIndividuals as System
from TravelAndMutate.trees import TreeBalanceProxy
import TravelAndMutate.datamanager as datman

def main(working_dir, seed, suppress_output=False):

	if not working_dir[-1] == "/":
		working_dir = working_dir + "/"

	with open(working_dir+"params.json") as paramfile:
		params_dict = json.load(paramfile)

	random_engine = NumpyRandomGenerator(seed)

	params = Params(params_dict, random_engine.rng).__dict__
	patch_params = pd.DataFrame()
	patch_params["N"] = params["Ns"].astype("u4")
	patch_params["beta"] = params["betas"]
	patch_params["epsilon"] = params["epsilons"]
	patch_params["mu"] = params["mus"]
	patch_params["I0"] = params["I0"].astype("u4")

	sys = System(random_engine.cpprng, params["commuting"], patch_params.to_records(index=False))

	starttime = time.time()
	sys.spreadForTime(params["t_max"])
	endtime = time.time()

	treeTB = sys.getTreeBalance()

	sim_attrs = {
		"seed" : seed,
		"exec_time" : endtime - starttime,
		"internals" : treeTB.internals(),
		"tips" : treeTB.tips()
	}
	datman.create_dataset(working_dir+"treesTB", params["params_dict"], seed, treeTB.tree(), sim_attrs, suppress_output=suppress_output)

	if not suppress_output:
		print(f"Time elapsed: {round(endtime-starttime, 2)} s")

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--seed", type=int, required=True)
	args = parser.parse_args()
	working_dir = args.dir
	seed = args.seed
	main(working_dir, seed)