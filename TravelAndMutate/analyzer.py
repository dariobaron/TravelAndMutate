import h5py
from TravelAndMutate.datamanager import checkAttributes, checkIsH5Group


def writeDataset(outfilename, groupname, datasetname, attributes, data):
	with h5py.File(outfilename, 'a') as outfile:
		if groupname in outfile:
			group = checkIsH5Group(outfile[groupname])
			try:
				checkAttributes(group, attributes)
			except:
				outfile.close()
				raise
		else:
			group = outfile.create_group(groupname, track_order=True)
			for key,val in attributes.items():
				group.attrs[key] = val
		if datasetname in group:
			del group[datasetname]
		dataset = group.create_dataset(datasetname, data=data, compression="gzip", compression_opts=9)
	return