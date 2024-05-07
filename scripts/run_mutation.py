import sys
import os
sys.path[0] = os.getcwd()

import json
import time
import pandas as pd
import numpy as np
from numpy.lib import recfunctions
from argparse import ArgumentParser
from TravelAndMutate.paramsmanager import Params
from TravelAndMutate.randominterface import NumpyRandomGenerator
from TravelAndMutate.haplotypes import Haplotypes
from TravelAndMutate.system import SystemMutations as System
import TravelAndMutate.datamanager as datman

def main(working_dir, filename, seed, suppress_output=False):

	if not working_dir[-1] == "/":
		working_dir = working_dir + "/"

	with open(working_dir+filename+".json") as paramfile:
		params_dict = json.load(paramfile)

	random_engine = NumpyRandomGenerator(seed)
	
	params = Params(params_dict, random_engine.rng).__dict__
	patch_params = pd.DataFrame()
	patch_params["N"] = params["Ns"].astype("u4")
	patch_params["beta"] = params["betas"]
	patch_params["epsilon"] = params["epsilons"]
	patch_params["mu"] = params["mus"]
	patch_params["I0"] = params["I0"].astype("u4")

	dealer = Haplotypes(random_engine.cpprng, params["mutation_rate"])

	system = System(random_engine.cpprng, params["commuting"], patch_params.to_records(index=False), params["gamma_trick"])
	system.setHaplotypes(dealer)
	system.seedEpidemic()
	system.setVerbosity(not suppress_output)

	starttime = time.time()
	system.spreadForTime(params["t_max"])
	simulationtime = time.time() - starttime

	starttime = time.time()
	mutations = recfunctions.stack_arrays(
		[system.getMutationTree(i) for i in range(params["N_patches"])],
		defaults=None, usemask=False, asrecarray=True, autoconvert=False
	)
	mutations.sort(order="t")
	sim_attrs = {
		"seed" : seed,
		"exec_time" : simulationtime
	}
	trajectories = [system.getFullTrajectory(p) for p in range(params["N_patches"])]
	haplotree = dealer.getMutationTree()
	unique_haplos = np.unique(mutations["mut"])
	unique_haplos.sort()
	sequences = dealer.read(unique_haplos)
	postprocesstime = time.time() - starttime

	starttime = time.time()
	group_identifier = datman.createReplica(working_dir+filename, params, seed, sim_attrs, suppress_output=suppress_output)
	datman.writeDatasetInGroup("infections", mutations, group_identifier, suppress_output)
	traj_identifier = datman.writeGroupInGroup("trajectories", group_identifier)
	for i,trajectory in enumerate(trajectories):
		datman.writeDatasetInGroup(str(i), trajectory, traj_identifier, suppress_output)
	datman.writeDatasetInGroup("mutationtree", haplotree, group_identifier, suppress_output)
	datman.writeDatasetInGroup("sequences", sequences, group_identifier, suppress_output)
	storingtime = time.time() - starttime

	if not suppress_output:
		print(f"Time elapsed simulating: {round(simulationtime, 2)} s")
		print(f"Time elapsed post-processing: {round(postprocesstime, 2)} s")
		print(f"Time elapsed storing data: {round(storingtime, 2)} s")

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--seed", type=int, required=True)
	parser.add_argument("--suppressoutput", type=bool, default=False)
	args = parser.parse_args()
	working_dir = args.dir
	filename = args.name
	seed = args.seed
	suppress_output = args.suppressoutput
	main(working_dir, filename, seed, suppress_output)