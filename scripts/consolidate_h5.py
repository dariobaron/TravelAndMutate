from ast import arg
import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
from TravelAndMutate.datamanager import consolidateH5

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--dir", type=str, required=True)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--inputnamebase", type=str, default="")
	args = parser.parse_args()
	directory = args.dir
	if directory[-1] != "/":
		directory = directory + "/"
	outputfilename = directory + args.name
	if args.inputnamebase == "":
		inputnamebase = outputfilename
	consolidateH5(inputnamebase, outputfilename)