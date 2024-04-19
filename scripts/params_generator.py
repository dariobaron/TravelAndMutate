import sys
import os
sys.path[0] = os.getcwd()

import json
from argparse import ArgumentParser

parser = ArgumentParser(allow_abbrev=False)
parser.add_argument("--dir", type=str, required=True)
args = parser.parse_args()

outputfolder = args.dir
if outputfolder[-1] != "/":
	outputfolder = outputfolder + "/"

params = {
	"sys_type" : "mutations",
	"N_patches" : 107,
	"t_max" : 10000,
	"mutation_rate" : 0.005,
	"Ns_setter" : "fromcsv",
	"Ns_params" : "inputparams/Ns/italy_small.csv",
	"commuting_setter" : "gravity",
	"commuting_params" : [1e-3, 0.46, 0.64, 82, "inputparams/distances/italy_small.csv"],
	"betas_setter" : "delta",
	"betas_params" : 0.03,
	"epsilons_setter" : "delta",
	"epsilons_params" : 0.1,
	"mus_setter" : "delta",
	"mus_params" : 0.02,
	"I0_setter" : "onehot",
	"I0_params" : [27,1]
}

with open(outputfolder+"params.json", "w") as outfile:
	json.dump(params, outfile)