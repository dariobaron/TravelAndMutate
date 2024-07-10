import attr
import pandas as pd
import numpy as np
import h5py
import os
from os.path import isfile
from tqdm import tqdm
import glob


def checkIsH5Dataset(dataset):
	if not isinstance(dataset, h5py.Dataset):
		raise RuntimeError("This is not a dataset")
	return dataset


def filterGroupmembersWithParams(group, params_dict):
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
	

def createNewGroupname(group, prefix="", postfix=""):
	members = list(group.keys())
	i = 0
	while(True):
		if prefix+str(i)+postfix not in members:
			return prefix+str(i)+postfix
		i = i + 1
		

def createReplica(filename, groupname, params_dict, seed, sim_attrs={}, suppress_output=False):
	outfilename = filename + f"_{groupname}_seed-{seed:05d}" + ".h5"
	if isfile(outfilename):
		if not suppress_output:
			print(f"WARNING: overwriting dataset {groupname} with seed {seed}")
	with h5py.File(outfilename, "w") as datafile:
		group = datafile.create_group(str(groupname), track_order=True)
		if not suppress_output:
			print("Writing attributes...", flush=True, end="\r")
		attributes = group.attrs
		for key,val in params_dict.items():
			attributes.create(key,val)
		replicaname = f"seed-{seed:05d}"
		replica = group.create_group(replicaname, track_order=True)
		for key,val in sim_attrs.items():
			replica.attrs.create(key,val)
		group_identifier = {"whichfile":outfilename, "whichgroup":replica.name}
	return group_identifier


def writeGroupInGroup(groupname, group_identifier):
	with h5py.File(group_identifier["whichfile"], "a") as datafile:
		group = datafile.require_group(group_identifier["whichgroup"])
		newgroup = group.create_group(str(groupname), track_order=True)
		newgroup_identifier = {"whichfile":group_identifier["whichfile"], "whichgroup":newgroup.name}
	return newgroup_identifier


def writeDatasetInGroup(datasetname, values, group_identifier, suppress_output=False):
	with h5py.File(group_identifier["whichfile"], "a") as datafile:	
		if not suppress_output:
			print("Writing dataset...", flush=True, end="\r")
		group = datafile.require_group(group_identifier["whichgroup"])
		dataset = group.create_dataset(datasetname, data=values, compression="gzip", compression_opts=9)


def collectAttributeFromGroup(key, group, applyfunc=None):
	if applyfunc is None:
		attributes = {name : element.attrs[key] for name,element in group.items() if key in element.attrs}
	else:
		attributes = {name : applyfunc(element.attrs[key]) for name,element in group.items() if key in element.attrs}
	try:
		length = len(next(iter(attributes.values())))
	except:
		length = 1
	columns = [key] * length
	df = pd.DataFrame.from_dict(attributes, orient="index", columns=columns)
	dropped = df.T.drop_duplicates()
	if dropped.shape[0] == 1:
		df = dropped.T
	return df


def getHeteroAttributes(group):
	df = pd.DataFrame.from_dict({name:dict(subgroup.attrs) for name,subgroup in group.items()}, orient="index")
	heteroattributes = []
	for attr,values in df.items():
		uniques = np.unique(np.array(df[attr].tolist()), axis=0).shape[0]
		if uniques != 1:
			heteroattributes.append(attr)
	df = pd.concat([collectAttributeFromGroup(attr, group) for attr in heteroattributes], axis=1)
	return df


def recursivelyCopyAttributes(srcgrp, destgrp):
	for key,val in srcgrp.attrs.items():
		destgrp.attrs.create(key,val)
	if isinstance(srcgrp, h5py.Group):
		for key in srcgrp.keys():
			recursivelyCopyAttributes(srcgrp[key], destgrp[key])


def checkAttributes(group, params_dict):
	if len(params_dict) != len(group.attrs):
		raise RuntimeError(f"Wrong attributes match: {group.name} does not have {len(params_dict)} attributes")
	for key,val in params_dict.items():
		if np.any(val != group.attrs[key]):
			raise RuntimeError(f"Attribute value of {key} in {group.name} mismatch the required value {val}")
	return


def consolidateH5(inputbasename, outfilename, suppress_output=False):
	infilenames = sorted(glob.glob(inputbasename+"_*_seed-*.h5"))
	toremove = []
	groupnames = [f.split("_")[-2] for f in infilenames]
	overwritten = []
	for i,infilename in enumerate(tqdm(infilenames, ncols=80, leave=False)):
		infile = h5py.File(infilename)
		outfile = h5py.File(outfilename, "a")
		try:
			ingroup = infile.require_group(groupnames[i])
			params_dict = dict(ingroup.attrs)
			groupname = filterGroupmembersWithParams(outfile, params_dict)
			if groupname is None:
				infile.copy(infile[groupnames[i]], outfile, name=groupnames[i], expand_soft=True, expand_external=True, expand_refs=True, without_attrs=True)
				recursivelyCopyAttributes(infile[groupnames[i]], outfile[groupnames[i]])
				toremove.append(infilename)
			elif groupname == groupnames[i]:
				outgroup = outfile.require_group(groupname)
				checkAttributes(ingroup, outgroup.attrs)
				subgroupname = list(ingroup.keys())[0]
				if subgroupname in outgroup:
					overwritten.append(subgroupname)
					del outgroup[subgroupname]
				infile.copy(ingroup[subgroupname], outgroup, name=subgroupname, expand_soft=True, expand_external=True, expand_refs=True, without_attrs=True)
				recursivelyCopyAttributes(ingroup[subgroupname], outgroup[subgroupname])
				toremove.append(infilename)
			else:
				raise RuntimeError(f"Group-name mismatch ({groupname} vs {groupnames[i]}) for params: {params_dict}")
			infile.close()
			outfile.close()
		except:
			infile.close()
			outfile.close()
			raise
	for f in toremove:
		os.remove(f)
	if len(overwritten) > 0:
		print(f"WARNING: overwritten {len(overwritten)} datasets!")
