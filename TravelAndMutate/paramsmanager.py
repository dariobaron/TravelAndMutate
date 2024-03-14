import numpy as np

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
		for key,val in setter_parameters.items():
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
	
