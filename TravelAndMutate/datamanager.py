import pandas as pd
import numpy as np
import h5py
import os
from os.path import isfile
from tqdm import tqdm
import glob


def filter_groupmembers_with_params(group, params_dict):
	names = list(group.keys())
	for name,member in group.items():
		for key,val in params_dict.items():
			if np.any(val != member.attrs[key]):
				names.remove(name)
				break
	if len(names) > 1:
		return names
	if len(names) == 1:
		return names[0]
	else:
		return None
	

def create_new_groupname(group, prefix="", postfix=""):
	members = list(group.keys())
	i = 0
	while(True):
		if prefix+str(i)+postfix not in members:
			return prefix+str(i)+postfix
		i = i + 1
		

def create_dataset(filename, params_dict, seed, values, sim_attrs={}, suppress_output=False):
	if isfile(filename+".h5"):
		with h5py.File(filename+".h5") as storage:
			groupname = filter_groupmembers_with_params(storage, params_dict)
			if groupname is None:
				groupname = create_new_groupname(storage)
	else:
		groupname = "0"
	if not isinstance(groupname, str):
		raise RuntimeError(f"Found multiple groups with same parameters: {groupname}")
	outfilename = filename + f"_{groupname}_seed-{seed:05d}" + ".h5"
	if isfile(outfilename):
		if not suppress_output:
			print(f"WARNING: overwriting dataset {groupname} with seed {seed}")
	with h5py.File(outfilename, "w") as datafile:
		group = datafile.create_group(groupname, track_order=True)
		attributes = group.attrs
		for key,val in params_dict.items():
			attributes.create(key,val)
		datasetname = f"seed-{seed:05d}"
		dataset = group.create_dataset(datasetname, data=values, compression="gzip", compression_opts=9)
		for key,val in sim_attrs.items():
			dataset.attrs.create(key,val)					


def collect_attribute_from_group(key, group, applyfunc=None):
	if applyfunc is None:
		attributes = {int(string.replace("seed-","")):dataset.attrs[key] for string,dataset in group.items() if key in dataset.attrs}
	else:
		attributes = {int(string.replace("seed-","")):applyfunc(dataset.attrs[key]) for string,dataset in group.items() if key in dataset.attrs}
	return pd.Series(attributes)


def consolidate_h5(inputfilename, outputfilename, suppress_output=False):
	infilenames = sorted(glob.glob(inputfilename+f"_*_seed-*.h5"))
	groupnames = [f.split("_")[-2] for f in infilenames]
	outfilename = outputfilename + ".h5"
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
