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
from TravelAndMutate.recorder import RecorderMutations as Recorder
from TravelAndMutate.sequencer import Sequencer
from TravelAndMutate.system import SystemMutations as System
import TravelAndMutate.datamanager as datman

def main(working_dir, filename, groupname, seed, suppress_output=False):

	if not working_dir[-1] == "/":
		working_dir = working_dir + "/"

	with open(f"{working_dir+filename}_{groupname}.json") as paramfile:
		params_dict = json.load(paramfile)

	random_engine = NumpyRandomGenerator(seed)
	
	params = Params(params_dict, random_engine.rng)
	patch_params = pd.DataFrame()
	patch_params["N"] = params["Ns"].astype("u4")
	patch_params["beta"] = params["betas"] * params["dt"]
	patch_params["epsilon"] = params["epsilons"] * params["dt"]
	patch_params["mu"] = params["mus"] * params["dt"]
	patch_params["I0"] = params["I0"].astype("u4")

	recorder = Recorder(params["N_patches"])

	haploproperties = {
		"mutation_rate":params["mutation_rate"]*params["dt"], "mutation_k":params["mutation_k"],
		"fitness_p":params["fitness_p"], "fitness_delta-":params["fitness_delta-"], "fitness_delta+":params["fitness_delta+"]
	}
	dealer = Haplotypes(random_engine.cpprng, haploproperties)

	sequencer = Sequencer(random_engine.cpprng, params["sequencing_prob"]*params["reporting_prob"], round(params["sequencing_delay"]/params["dt"]))

	system = System(random_engine.cpprng, params["commuting"], patch_params.to_records(index=False), params["gamma_trick"])
	system.setRecorder(recorder)
	system.setHaplotypes(dealer)
	system.setSequencer(sequencer)
	system.seedEpidemic()
	system.setVerbosity(not suppress_output)

	starttime = time.time()
	system.spreadForTime(params["t_max"])
	simulationtime = time.time() - starttime

	starttime = time.time()
	infections = recorder.getInfectionTree()
	infections["t"] = np.round(infections["t"]*params["dt"])
	infections = recfunctions.drop_fields(infections, ["ID","inf_ID","inf_mut"])
	trajectories = [recorder.getFullTrajectory(p) for p in range(params["N_patches"])]
	for trajectory in trajectories:
		trajectory["t"] = np.round(trajectory["t"]*params["dt"])
	haplotree = dealer.getMutationTree()
	fitness = dealer.getAllPhi()
	sampled = sequencer.getSampledIDs()
	sampled["t"] = np.round(sampled["t"]*params["dt"])
	sim_attrs = {
		"seed" : seed,
		"exec_time" : simulationtime,
		"survived" : np.mean(
						[trajectory["R"][-1] > I0
						for trajectory,I0 in zip(trajectories,patch_params["I0"])]
					) >= 0.2
	}
	sim_attrs.update(params.getSimParams())
	postprocesstime = time.time() - starttime

	starttime = time.time()
	group_identifier = datman.createReplica(working_dir+filename, groupname, params.getGroupParams(), seed, sim_attrs, suppress_output=suppress_output)
	datman.writeDatasetInGroup("infections", infections, group_identifier, suppress_output)
	traj_identifier = datman.writeGroupInGroup("trajectories", group_identifier)
	for i,trajectory in enumerate(trajectories):
		datman.writeDatasetInGroup(str(i), trajectory, traj_identifier, suppress_output)
	datman.writeDatasetInGroup("mutationtree", haplotree, group_identifier, suppress_output)
	datman.writeDatasetInGroup("fitness", fitness, group_identifier, suppress_output)
	datman.writeDatasetInGroup("sequencings", sampled, group_identifier, suppress_output)
	storingtime = time.time() - starttime

	if not suppress_output:
		print(f"Time elapsed simulating: {round(simulationtime, 2)} s")
		print(f"Time elapsed post-processing: {round(postprocesstime, 2)} s")
		print(f"Time elapsed storing data: {round(storingtime, 2)} s")

	return infections.shape[0]


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--group", type=str, required=True)
	parser.add_argument("--seed", type=int, required=True)
	parser.add_argument("--suppressoutput", type=bool, default=False)
	args = parser.parse_args()
	working_dir = args.dir
	filename = args.name
	groupname = args.group
	seed = args.seed
	suppress_output = args.suppressoutput
	main(working_dir, filename, groupname, seed, suppress_output)