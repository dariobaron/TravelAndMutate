import numpy as np
import h5py

class Params:
	def __init__(self, params_dict, rng):
		self.params_dict = params_dict
		self.rng = rng
		updater = {key:val for key,val in params_dict.items() if "_setter" not in key and "_params" not in key}
		self.__dict__.update(updater)
		setter_parameters = {key.replace("_setter",""):val for key,val in params_dict.items() if "_setter" in key}
		params_parameters = {key.replace("_params",""):val for key,val in params_dict.items() if "_params" in key}
		if list(setter_parameters.keys()) != list(params_parameters.keys()):
			raise ValueError(f"Parameters types and params mismatch:\n{list(setter_parameters.keys())}\nvs\n{list(params_parameters.keys())}")
		updater = {}
		try:
			updater["Ns"] = getattr(self, setter_parameters["Ns"])(*params_parameters["Ns"])
		except:
			updater["Ns"] = getattr(self, setter_parameters["Ns"])(params_parameters["Ns"])
		for key,val in setter_parameters.items():
			if key == "Ns":
				continue
			func = getattr(self, val)
			try:
				updater[key] = func(*params_parameters[key])
			except:
				updater[key] = func(params_parameters[key])
		self.__dict__.update(updater)

	def provided(self, *values):
		return np.array(values)
	
	def delta(self, value):
		return np.full(self.N_patches, value)
	
	def onehot(self, idx, value):
		arr = np.zeros(self.N_patches)
		arr[idx] = value
		return arr

	def fromtxt(self, filename):
		return np.loadtxt(filename)
	
	def fromh5(self, filename, pathtodataset):
		with h5py.File(filename, "r") as inputfile:
			values = inputfile[pathtodataset]
		return values

	def gravity(self, scale, alpha, gamma, r, *positions):
		pos = np.array(positions).reshape((self.N_patches,2))
		c_ij = np.empty((self.N_patches,self.N_patches))
		for i in range(self.N_patches):
			for j in range(self.N_patches):
				if i == j:
					c_ij[i,j] = 1
				else:
					c_ij[i,j] = self.Ns[j]**gamma * self.Ns[i]**(alpha-1) / np.exp(1/r*np.linalg.norm(pos[i]-pos[j]))
		outdiagmean = (np.sum(c_ij) - np.sum(np.diag(c_ij))) / (self.N_patches - 1)**2
		for i in range(self.N_patches):
			for j in range(self.N_patches):
				c_ij[i,j] = c_ij[i,j] / outdiagmean * scale
		return c_ij
	
