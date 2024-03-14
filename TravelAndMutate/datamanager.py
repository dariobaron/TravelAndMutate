import pandas as pd
import numpy as np
import h5py
import os
from os.path import isfile
from tqdm import tqdm


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
		group = datafile.require_group(groupname)
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


