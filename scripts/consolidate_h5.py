import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
import glob
from tqdm import tqdm
import h5py
import multiprocessing as mp
from TravelAndMutate.datamanager import recursivelyCopyAttributes, checkAttributes, checkIsH5Group


class Kernel:
	def __init__(self, outfilename):
		self.outfilename = outfilename
	def __call__(self, tpl):
		groupname, filenamelist = tpl
		with h5py.File(self.outfilename) as outfile:
			if groupname not in outfile:
				return (None, [None])
			attrs = dict(outfile[groupname].attrs)
			compliance = []
			for i,infilename in enumerate(filenamelist):
				with h5py.File(infilename) as infile:
					try:
						checkAttributes(infile[groupname], attrs)
						compliance.append(True)
					except:
						compliance.append(False)
		return (groupname, compliance)



def main(inputbasename, outfilename, nprocs, suppress_output=False):
	names = sorted(glob.glob(inputbasename+"_*_seed-*.h5"))
	infilenames = {}
	for name in tqdm(names, leave=False):
		groupname = name.split("_")[-2]
		if groupname in infilenames:
			infilenames[groupname].append(name)
		else:
			infilenames[groupname] = [name]
	errors = []
	kernel = Kernel(outfilename)
	if nprocs > 1:
		with mp.Pool(nprocs-1) as workers:
			results = workers.imap_unordered(kernel, infilenames.items())
			for groupname,compliance in tqdm(results, total=len(infilenames.keys()), desc="Checking attributes integrity", dynamic_ncols=True):
				if groupname is None:
					print(f"Group {groupname} does not exist in the output file")
					errors.extend(infilenames[groupname])
					infilenames[groupname] = []
				else:
					errors.extend([filename for i,filename in enumerate(infilenames[groupname]) if not compliance[i]])
					infilenames[groupname] = [filename for i,filename in enumerate(infilenames[groupname]) if compliance[i]]
	else:
		for tpl in tqdm(infilenames.items(), desc="Checking attributes integrity", dynamic_ncols=True):
			groupname,compliance = kernel(tpl)
			if groupname is None:
				print(f"Group {groupname} does not exist in the output file")
				errors.extend([f"Group {groupname} does not exists in the output file"])
				infilenames[groupname] = []
			else:
				errors.extend([filename for i,filename in enumerate(infilenames[groupname]) if not compliance[i]])
				infilenames[groupname] = [filename for i,filename in enumerate(infilenames[groupname]) if compliance[i]]

	overwritten = []
	for groupname,namelist in tqdm(infilenames.items(), desc="Copying files", dynamic_ncols=True):
		toremove = []
		outfile = h5py.File(outfilename, "a")
		outgroup = checkIsH5Group(outfile[groupname])
		try:
			for infilename in namelist:
				with h5py.File(infilename) as infile:
					ingroup = checkIsH5Group(infile[groupname])
					runname = list(ingroup.keys())[0]
					if runname in outgroup:
						overwritten.append(runname)
						del outgroup[runname]
					infile.copy(ingroup[runname], outgroup, name=runname, expand_soft=True, expand_external=True, expand_refs=True, without_attrs=True)
					recursivelyCopyAttributes(ingroup[runname], outgroup[runname])
					toremove.append(infilename)
		except:
			outfile.close()
			raise
		outfile.close()
		for f in toremove:
			os.remove(f)
	if len(errors) > 0:
		print(f"Errors in the attributes were encountered for the files: {errors}")
	if len(overwritten) > 0:
		print(f"WARNING: overwritten {len(overwritten)} datasets!")


if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--outfile", type=str, required=True)
	parser.add_argument("--inputnamebase", type=str, default="")
	parser.add_argument("--nprocs", type=int, default=0)
	args = parser.parse_args()
	outfilename = args.outfile
	if outfilename[-3:] != ".h5":
		raise RuntimeError("Output filename must end with '.h5'")
	if not os.path.isfile(outfilename):
		print(f"{outfilename} does not exist. Creating it!")
	if args.inputnamebase == "":
		inputnamebase = outfilename.replace(".h5", "")
	nprocs = args.nprocs
	main(inputnamebase, outfilename, nprocs)