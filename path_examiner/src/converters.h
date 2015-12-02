#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <boost/config.hpp>
#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>

namespace kayrebt
{

	class PredecessorCollection : public std::vector<unsigned int>
	{
		using BaseType = std::vector<unsigned int>;
		using BaseType::BaseType;
	};

	class ArgCollection : public std::vector<std::string>
	{
		using BaseType = std::vector<std::string>;
		using BaseType::BaseType;
	};


	std::ostream& operator<<(std::ostream& out, const PredecessorCollection& arg)
	{
		std::copy(arg.cbegin(), arg.cend(), std::ostream_iterator<unsigned int>(out,","));
		return out;
	}
	std::ostream& operator<<(std::ostream& out, const ArgCollection& arg)
	{
		std::copy(arg.cbegin(), arg.cend(), std::ostream_iterator<std::string>(out,","));
		return out;
	}

	bool operator>>(std::istream& it, PredecessorCollection& result)
	{
		while (it && !it.eof()) {
			unsigned int buf;
			it >> buf;
			if (it) {
				result.push_back(buf);
				char c = it.get();
				if (c != ',')
					break;
			}
		}
		return it.eof();
	}

	bool operator>>(std::istream& it, ArgCollection& result)
	{
		while (it && !it.eof()) {
			std::string buf;
			std::getline(it, buf, ',');
			if (it)
				result.push_back(std::move(buf));
		}
		return it.eof();
	}

}

#endif
