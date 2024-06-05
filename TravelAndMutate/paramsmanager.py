from matplotlib.backend_bases import key_press_handler
import numpy as np
import pandas as pd
import h5py

class Params:
	def __init__(self, params_dict, rng=None):
		self.group = {}
		self.sim = {}
		# dummy inits
		self.N_patches = 0
		self.Ns = []
		# single-valued parameters
		updater = {key:val for key,val in params_dict.items() if "_setter" not in key and "_params" not in key}
		self.__dict__.update(updater)
		self.group.update(updater)
		# multi-valued parameters
		setter_parameters = {key.replace("_setter",""):val for key,val in params_dict.items() if "_setter" in key}
		params_parameters = {key.replace("_params",""):val for key,val in params_dict.items() if "_params" in key}
		if list(setter_parameters.keys()) != list(params_parameters.keys()):
			raise ValueError(f"Parameters types and params mismatch:\n{list(setter_parameters.keys())}\nvs\n{list(params_parameters.keys())}")
		### manually imposing precedence on dependent parameters
		precedence = ["Ns"]
		updater = {}
		for key in precedence:
			val = setter_parameters[key]
			func = getattr(self, val)
			if val[:4] == "rnd_":
				try:
					updater[key] = func(rng, *params_parameters[key])
				except:
					updater[key] = func(rng, params_parameters[key])
				self.__dict__.update(updater)
				self.sim.update(updater)
			else:
				try:
					updater[key] = func(*params_parameters[key])
				except:
					updater[key] = func(params_parameters[key])
				self.__dict__.update(updater)
				self.group.update(updater)
			updater = {}
		### updating all the other parameters
		for key,val in setter_parameters.items():
			if key in precedence:
				continue
			func = getattr(self, val)
			if val[:4] == "rnd_":
				try:
					updater[key] = func(rng, *params_parameters[key])
				except:
					updater[key] = func(rng, params_parameters[key])
				self.__dict__.update(updater)
				self.sim.update(updater)
			else:
				try:
					updater[key] = func(*params_parameters[key])
				except:
					updater[key] = func(params_parameters[key])
				self.__dict__.update(updater)
				self.group.update(updater)
			updater = {}

	def __getitem__(self, arg):
		return self.__dict__[arg]

	def getGroupParams(self):
		return self.group
	
	def getSimParams(self):
		return self.sim

	def provided(self, *values):
		return np.array(values)
	
	def delta(self, value):
		return np.full(self.N_patches, value)
	
	def onehot(self, idx, value):
		arr = np.zeros(self.N_patches)
		arr[idx] = value
		return arr

	def fromcsv(self, filename):
		return pd.read_csv(filename).to_numpy().squeeze()
	
	def fromh5(self, filename, pathtodataset):
		with h5py.File(filename, "r") as inputfile:
			values = inputfile[pathtodataset]
			if isinstance(values, h5py.Dataset):
				values = values[:]
			else:
				raise RuntimeError(f"{pathtodataset} in H5File {filename} is not a dataset")
		return values
