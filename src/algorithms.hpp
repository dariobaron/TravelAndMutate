#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>

template<typename T>
void eraseWithoutOrder(std::vector<T> & v, unsigned index){
	v[index] = std::move(v.back());
	v.pop_back();
}
template<typename T>
void eraseWithoutOrder(std::vector<T> & v, std::vector<unsigned> indices){
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

#endif