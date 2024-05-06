from ast import arg
import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
from TravelAndMutate.datamanager import consolidateH5

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--outputfilename", type=str, required=True)
	parser.add_argument("--inputfilename", type=str, default="")
	parser.add_argument("--dir", type=str, default="")
	args = parser.parse_args()
	directory = args.dir
	outputfilename = directory + args.outputfilename
	if args.inputfilename == "":
		inputfilename = outputfilename
	else:
		inputfilename = directory + args.inputfilename
	consolidateH5(inputfilename, outputfilename)