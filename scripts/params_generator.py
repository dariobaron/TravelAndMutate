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
	"sys_type" : "individuals",
	"N_patches" : 10,
	"t_max" : 10000,
	"Ns_setter" : "provided",
	"Ns_params" : [3000,3000,1000,1000,1000,1000,500,500,500,500],
	"commuting_setter" : "provided",
	"commuting_params" : [[1.0, 0.001, 0.01, 0.01, 0.01, 0.01, 0.0, 0.0, 0.0, 0.0],
 [0.001, 1.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1],
 [0.01, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
 [0.01, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
 [0.01, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
 [0.01, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0],
 [0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0],
 [0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0],
 [0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0],
 [0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0]],
	"betas_setter" : "delta",
	"betas_params" : 0.02,
	"epsilons_setter" : "delta",
	"epsilons_params" : 0.1,
	"mus_setter" : "delta",
	"mus_params" : 0.01,
	"I0_setter" : "onehot",
	"I0_params" : [0,1]
}

with open(outputfolder+"params.json", "w") as outfile:
	json.dump(params, outfile)