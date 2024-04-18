import numpy as np
import pandas as pd
import h5py

class Params:
	def __init__(self, params_dict, rng):
		# dummy inits
		self.N_patches = 0
		self.Ns = []
		# single-valued parameters
		updater = {key:val for key,val in params_dict.items() if "_setter" not in key and "_params" not in key}
		self.__dict__.update(updater)
		# multi-valued parameters
		setter_parameters = {key.replace("_setter",""):val for key,val in params_dict.items() if "_setter" in key}
		params_parameters = {key.replace("_params",""):val for key,val in params_dict.items() if "_params" in key}
		if list(setter_parameters.keys()) != list(params_parameters.keys()):
			raise ValueError(f"Parameters types and params mismatch:\n{list(setter_parameters.keys())}\nvs\n{list(params_parameters.keys())}")
		### manually imposing precedence on dependent parameters
		updater = {}
		try:
			updater["Ns"] = getattr(self, setter_parameters["Ns"])(rng, *params_parameters["Ns"])
		except:
			updater["Ns"] = getattr(self, setter_parameters["Ns"])(rng, params_parameters["Ns"])
		self.__dict__.update(updater)
		### updating all the other parameters
		updater = {}
		for key,val in setter_parameters.items():
			if key == "Ns":
				continue
			func = getattr(self, val)
			try:
				updater[key] = func(rng, *params_parameters[key])
			except:
				updater[key] = func(rng, params_parameters[key])
		self.__dict__.update(updater)


	def provided(self, rng, *values):
		return np.array(values)
	
	def delta(self, rng, value):
		return np.full(self.N_patches, value)
	
	def onehot(self, rng, idx, value):
		arr = np.zeros(self.N_patches)
		arr[idx] = value
		return arr

	def fromcsv(self, rng, filename):
		return pd.read_csv(filename).to_numpy().squeeze()
	
	def fromh5(self, rng, filename, pathtodataset):
		with h5py.File(filename, "r") as inputfile:
			values = inputfile[pathtodataset]
			if isinstance(values, h5py.Dataset):
				values = values[:]
			else:
				raise RuntimeError(f"{pathtodataset} in H5File {filename} is not a dataset")
		return values

	def gravity(self, rng, scale, alpha, gamma, r, file_of_distances):
		dist = self.fromcsv(rng, file_of_distances)
		c_ij = np.empty((self.N_patches,self.N_patches))
		for i in range(self.N_patches):
			for j in range(self.N_patches):
				if i == j:
					c_ij[i,j] = 1
				else:
					c_ij[i,j] = self.Ns[j]**gamma * self.Ns[i]**(alpha-1) / np.exp(1/r*dist[i,j])
		outdiagmean = (np.sum(c_ij) - np.sum(np.diag(c_ij))) / (self.N_patches - 1)**2
		for i in range(self.N_patches):
			for j in range(self.N_patches):
				if i != j:
					c_ij[i,j] = c_ij[i,j] / outdiagmean * scale
		return c_ij
	
