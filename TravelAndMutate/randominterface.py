#! ../env/bin/python

import numpy as np
from TravelAndMutate.randomcore import RNGcore

class BitGenerator(np.random.BitGenerator):
	def __init__(self, seed=None):
		super().__init__()
		if seed is None:
			self.rng = RNGcore()
		else:
			self.rng = RNGcore(seed)
		self.rng.setup_numpy_capsule(self.capsule)
		


class NumpyRandomGenerator:
	def __init__(self, seed=None):
		self.rng = np.random.Generator(BitGenerator(seed))
		self.cpprng = self.rng.bit_generator.rng