import sys
import os
sys.path[0] = os.getcwd()

from argparse import ArgumentParser
import numpy as np
import pandas as pd


def main(name, scale, alpha, gamma, r):
	Ns = pd.read_csv(f"inputparams/Ns/{name}.csv").to_numpy().squeeze()
	distances = pd.read_csv(f"inputparams/distances/{name}.csv")
	columns = distances.columns
	distances = distances.to_numpy().squeeze()
	N_patches = Ns.shape[0]
	if (N_patches != distances.shape[0]) or (distances.shape[0] != distances.shape[1]):
		raise RuntimeError(f"Shapes of populations ({Ns.shape}) and distances ({distances.shape}) mismatch")
	c_ij = np.empty((N_patches,N_patches))
	for i in range(N_patches):
		for j in range(N_patches):
			if i == j:
				c_ij[i,j] = 1
			else:
				c_ij[i,j] = Ns[j]**gamma * Ns[i]**(alpha-1) / np.exp(1/r*distances[i,j])
	outdiagmean = (np.sum(c_ij) - np.sum(np.diag(c_ij))) / (N_patches - 1)**2
	for i in range(N_patches):
		for j in range(N_patches):
			if i != j:
				c_ij[i,j] = c_ij[i,j] / outdiagmean * scale
	pd.DataFrame(c_ij, columns=columns).to_csv(f"inputparams/c_ij/{name}.csv", index=False)

	

if __name__ == "__main__":
	parser = ArgumentParser(allow_abbrev=False)
	parser.add_argument("--name", type=str, required=True)
	parser.add_argument("--scale", type=float, default=1e-3)
	parser.add_argument("--alpha", type=float, default=0.46)
	parser.add_argument("--gamma", type=float, default=0.64)
	parser.add_argument("--r", type=float, default=82)
	args = parser.parse_args()
	name = args.name
	scale = args.scale
	alpha = args.alpha
	gamma = args.gamma
	r = args.r
	main(name, scale, alpha, gamma, r)