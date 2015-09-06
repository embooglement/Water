#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <vector>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <utility>

namespace util {
	template <typename ResultType, typename Type, typename Func>
	auto map(const std::vector<Type>& vec, Func&& f) -> std::vector<ResultType> {
		// typedef decltype(f(std::declval<Type>()) ResultType;
		std::vector<ResultType> result;
		std::transform(std::begin(vec), std::end(vec), std::back_inserter(result), f);
		return result;
	}
}

#endif
