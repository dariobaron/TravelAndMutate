import sys
import os
sys.path[0] = os.getcwd()

import h5py
import glob
from tqdm import tqdm
from argparse import ArgumentParser
from TravelAndMutate.datamanager import filter_groupmembers_with_params

def main(filename, suppress_output=False):
	infilenames = sorted(glob.glob(filename+f"_*_seed-*.h5"))
	groupnames = [f.split("_")[-2] for f in infilenames]
	outfilename = filename + ".h5"
	overwritten = []
	with h5py.File(outfilename, "a") as outfile:
		for i,infilename in enumerate(tqdm(infilenames, ncols=80, leave=False)):
			with h5py.File(infilename) as infile:
				ingroup = infile.require_group(groupnames[i])
				params_dict = dict(ingroup.attrs)
				groupname = filter_groupmembers_with_params(outfile, params_dict)
				if groupname is None:
					infile.copy(infile[groupnames[i]], outfile, name=groupnames[i], expand_soft=True, expand_external=True, expand_refs=True)
				elif groupname == groupnames[i]:
					outgroup = outfile.require_group(groupname)
					if len(ingroup.attrs) != len(outgroup.attrs):
						raise RuntimeError("Number of parameters of the simulation do not match with the ones for the existing group")
					datasetname = list(ingroup.keys())[0]
					if datasetname in outgroup:
						overwritten.append(datasetname)
						del outgroup[datasetname]
					infile.copy(ingroup[datasetname], outgroup, name=datasetname, expand_soft=True, expand_external=True, expand_refs=True)
				else:
					raise RuntimeError(f"Group-name mismatch ({groupname} vs {groupnames[i]}) for params: {params_dict}")
	for f in infilenames:
		os.remove(f)
	if len(overwritten) > 0:
		print(f"WARNING: overwritten {len(overwritten)} datasets!")

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--filename", type=str, required=True)
	args = parser.parse_args()
	filename = args.filename
	main(filename)