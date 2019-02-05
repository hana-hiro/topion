#include "topion.hpp"

struct MyOptions : public Topion{
	TOPION_ADD_MA(std::string, mode, "MODE", "Mode of the program");
	// MA: Mandatory + Argument name ("MODE")
	TOPION_ADD_ODA(std::size_t, trials, 1000, "UINT", "Number of trials");
	// ODA: Optional + Default value (1000) + Argument name ("UINT")
	TOPION_STANDALONE_ATLEAST(1, "[FILES...]");
};

#include <iostream>

int main(int argc, char ** argv){
	MyOptions mo;
	try{
		mo.topion_parse(argc, argv);
	}catch(const topion_parsing_error & e){
		std::cerr << e.what() << std::endl;
		mo.topion_usage(std::cerr);
		return -1;
	}
	std::cout << "Mode: " << mo.mode << std::endl;
	std::cout << "Trials: " << mo.trials << std::endl;
	for(size_t i = 0; i < mo.topion_standalone_size(); ++i){
		std::cout << "File[" << (i+1) << "/" << mo.topion_standalone_size() << "]: " << mo[i] << std::endl;
	}
	return 0;
}
