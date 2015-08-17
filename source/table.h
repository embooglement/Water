#ifndef _TABLE_H_
#define _TABLE_H_

#include <unordered_map>
#include <tuple>
#include <utility>

template <typename Key, typename... Values>
class table : public std::unordered_map<Key, std::tuple<Values...>> {
private:
	using _BaseType = typename std::unordered_map<Key, std::tuple<Values...>>;
public:
	using iterator = typename _BaseType::iterator;

	template <size_t I>
	auto get(const Key& key) -> decltype((std::get<I>((*this)[key]))) {
		return std::get<I>((*this)[key]);
	}
};

#endif
