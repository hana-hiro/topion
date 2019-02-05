#include "topion.hpp"
#include <iostream>
#include <stdexcept>

// A switch that accepts one character among specified ones
struct CharSwitch{
	const char * candidates;
	CharSwitch(const char * cand) : candidates(cand) {}

	bool operator()(void * target, const std::string & s) const{
		if(s.length() != 1) return false;
		*(static_cast<char *>(target)) = s[0];

		const char * c = candidates;
		while(*c != '\0'){
			if(s[0] == *c) return true;
			++c;
		}
		return false;
	}
};

// A switch that accepts values between two specified values
// Assumes that TopionUtil::Converter<T> is predefined
template<class T>
struct RangeSwitch{
	T minval, maxval;
	RangeSwitch(const T & a, const T & b) : minval(a), maxval(b) {}

	bool operator()(void * target, const std::string & s) const{
		if(!(TopionUtil::Converter<T>()(target, s))) return false;
		if(*(static_cast<T *>(target)) < minval || *(static_cast<T *>(target)) > maxval) return false;
		return true;
	}
};

struct Options : public Topion{
	TOPION_ADD_M(std::string, file, "Input file");
	TOPION_ADD_MC(double, rate, RangeSwitch<double>(0.0, 1.0), "Rate of increases (0 to 1)");
	TOPION_ADD_MA(int, i, "integer", "User ID");
	TOPION_ADD_MAC(short, size, "SIZE", RangeSwitch<short>(1, 1024), "Maximum size of data");
	TOPION_ADD_OS(bool, verbose, 'v', "Display detailed processes");
	TOPION_ADD_MSC(char, input, 'I', CharSwitch("xyz"), "Input type ('x', 'y' or 'z')");
	TOPION_ADD_MSA(unsigned int, trials, 'T', "UINT", "Number of trials");
	TOPION_ADD_MSAC(int, volume, 'V', "-100 to 100", RangeSwitch<int>(-100, 100), "Relative volume");
};

int main(int argc, char ** argv){
	Options o;
	try{
		o.topion_parse(argc, argv);
	}catch(const topion_parsing_error & e){
		std::cerr << e.what() << std::endl;
		o.topion_usage(std::cerr);
		return -1;
	}

	std::cout << "size = " << o.size << std::endl;
	std::cout << "i = " << o.i << std::endl;
	std::cout << "file = " << o.file << std::endl;
	std::cout << "rate = " << o.rate << std::endl;
	std::cout << "verbose = " << o.verbose << std::endl;
	std::cout << "input = " << o.input << std::endl;
	std::cout << "trials = " << o.trials << std::endl;
	std::cout << "volume = " << o.volume << std::endl;

	if(o.topion_standalone_size() > 0){
		std::cout << "Standalone parameters:" << std::endl;
		for(size_t i = 0; i < o.topion_standalone_size(); ++i){
			std::cout << (i+1) << ": " << o[i] << std::endl;
		}
	}
	return 0;
}
