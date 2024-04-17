from ast import arg
import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
from TravelAndMutate.datamanager import consolidate_h5

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--outputfilename", type=str, required=True)
	parser.add_argument("--inputfilename", type=str, default="")
	args = parser.parse_args()
	outputfilename = args.outputfilename
	if args.inputfilename == "":
		inputfilename = outputfilename
	else:
		inputfilename = args.inputfilename
	consolidate_h5(inputfilename, outputfilename)