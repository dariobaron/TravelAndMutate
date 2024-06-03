from ast import arg
import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
from TravelAndMutate.datamanager import consolidateH5

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--outfile", type=str, required=True)
	parser.add_argument("--inputnamebase", type=str, default="")
	args = parser.parse_args()
	outfilename = args.outfile
	if outfilename[-3:] != ".h5":
		raise RuntimeError("Output filename must end with '.h5'")
	if not os.path.isfile(outfilename):
		print(f"{outfilename} does not exist. Creating it!")
	if args.inputnamebase == "":
		inputnamebase = outfilename.replace(".h5", "")
	consolidateH5(inputnamebase, outfilename)