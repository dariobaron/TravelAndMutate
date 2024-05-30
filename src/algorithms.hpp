#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>
#include <random>

template<typename T>
void eraseWithoutOrder(std::vector<T> & v, unsigned index){
	v[index] = std::move(v.back());
	v.pop_back();
}
template<typename T>
void eraseWithoutOrder(std::vector<T> & v, std::vector<unsigned> indices){
	std::sort(indices.begin(), indices.end(), std::greater<unsigned>());
	for (auto i : indices){
		v[i] = std::move(v.back());
		v.pop_back();
	}
}
template<typename T, typename Iterator>
void eraseWithoutOrder(std::vector<T> & v, Iterator it){
	*it = std::move(v.back());
	v.pop_back();
}
template<typename T, typename Iterator>
void eraseWithoutOrder(std::vector<T> & v, Iterator beg, Iterator end){
	auto N = end - beg;
	std::move(v.end()-N, v.end(), beg);
	v.resize(v.size()-N);
}


template<typename RandomEngine>
std::vector<unsigned> sampleIndices(RandomEngine & rng, unsigned vec_size, unsigned n){
	std::vector<unsigned> all_indices(vec_size);
	std::iota(all_indices.begin(), all_indices.end(), 0);
	std::vector<unsigned> indices(n);
	std::sample(all_indices.begin(), all_indices.end(), indices.begin(), n, rng);
	return indices;
}


template<typename RandomEngine>
std::vector<unsigned> sampleIndicesWithReplacement(RandomEngine & rng, unsigned vec_size, unsigned n){
	std::uniform_int_distribution<unsigned> Distr(0, vec_size-1);
	std::vector<unsigned> indices(n);
	for (auto & i : indices){
		i = Distr(rng);
	}
	return indices;
}
template<typename RandomEngine>
std::vector<unsigned> sampleIndicesWithReplacement(RandomEngine & rng, const std::vector<double> & weights, unsigned n){
	std::discrete_distribution<unsigned> Distr(weights.begin(), weights.end());
	std::vector<unsigned> indices(n);
	for (auto & i : indices){
		i = Distr(rng);
	}
	return indices;
}


template<typename T>
void appendToEraseFromByIndices(std::vector<T> & target, std::vector<T> & source, const std::vector<unsigned> & indices){
	for (auto i : indices){
		target.push_back(std::move(source[i]));
	}
	eraseWithoutOrder(source, indices);
}


#endif