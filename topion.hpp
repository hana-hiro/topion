#ifndef TOPION_HPP_
#define TOPION_HPP_

//#define TOPION_DEBUG

#include <deque>
#include <unordered_map>
#include <functional>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <limits>

#ifdef TOPION_DEBUG
#include <iostream>
#endif // TOPION_DEBUG

class topion_definition_error: public std::invalid_argument {
	using std::invalid_argument::invalid_argument;
};
class topion_parsing_error: public std::runtime_error {
	using std::runtime_error::runtime_error;
};

namespace TopionUtil{
	using converter_type = std::function<bool(void *, const std::string &)>;

	template<class INTTYPE, class CONVFUNCTYPE>
	bool signed_int_converter(INTTYPE * pi, const std::string & s, const CONVFUNCTYPE & conv_func){
		char * ptr;
		*pi = conv_func(s.c_str(), &ptr, 10);
		for(;;){
			if(*ptr == '\0'){
				return true;
			}else if(std::isspace(*ptr)){
				++ptr;
			}else{
				return false;
			}
		}
	}

	template<class INTTYPE, class CONVFUNCTYPE>
	bool unsigned_int_converter(INTTYPE * pi, const std::string & s, const CONVFUNCTYPE & conv_func){
		size_t start_except_spaces = 0;
		for(; start_except_spaces < s.length(); ++start_except_spaces){
			if(!(std::isspace(s[start_except_spaces]))) break;
		}
		if(s[start_except_spaces] == '-') return false;

		char * ptr;
		*pi = conv_func(s.c_str() + start_except_spaces, &ptr, 10);
		for(;;){
			if(*ptr == '\0'){
				return true;
			}else if(std::isspace(*ptr)){
				++ptr;
			}else{
				return false;
			}
		}
	}

	template<class T>
	struct Converter{
		bool operator()(void *, const std::string &) const{
			// do nothing
			return false;
		}
	};

	template<>
	struct Converter<char>{
		bool operator()(void * target, const std::string & s) const{
			if(s.length() != 1) return false;
			*(static_cast<char *>(target)) = s[0];
			return true;
		}
	};

	template<>
	struct Converter<std::string>{
		bool operator()(void * target, const std::string & s) const{
			(static_cast<std::string *>(target))->assign(s);
			return true;
		}
	};

	template<>
	struct Converter<long long int>{
		bool operator()(void * target, const std::string & s) const{
			return signed_int_converter(static_cast<long long int *>(target), s, std::strtoll);
		}
	};

	template<>
	struct Converter<unsigned long long int>{
		bool operator()(void * target, const std::string & s) const{
			// Fail if the string begins with a minus sign
			return unsigned_int_converter(static_cast<unsigned long long int *>(target), s, std::strtoull);
		}
	};

	template<>
	struct Converter<long int>{
		bool operator()(void * target, const std::string & s) const{
			return signed_int_converter(static_cast<long int *>(target), s, std::strtol);
		}
	};

	template<>
	struct Converter<unsigned long int>{
		bool operator()(void * target, const std::string & s) const{
			return unsigned_int_converter(static_cast<unsigned long int *>(target), s, std::strtoul);
		}
	};

	template<>
	struct Converter<int>{
		bool operator()(void * target, const std::string & s) const{
			long int li;
			if(!Converter<long int>()(&li, s)) return false;

			*(static_cast<int *>(target)) = static_cast<int>(li);
			return(static_cast<long int>(*(static_cast<int *>(target))) == li);
		}
	};

	template<>
	struct Converter<unsigned int>{
		bool operator()(void * target, const std::string & s) const{
			unsigned long int uli;
			if(!Converter<unsigned long int>()(&uli, s)) return false;

			*(static_cast<unsigned int *>(target)) = static_cast<unsigned int>(uli);
			return(static_cast<unsigned long int>(*(static_cast<unsigned int *>(target))) == uli);
		}
	};

	template<>
	struct Converter<short>{
		bool operator()(void * target, const std::string & s) const{
			long int li;
			if(!Converter<long int>()(&li, s)) return false;

			*(static_cast<short *>(target)) = static_cast<short>(li);
			return(static_cast<long int>(*(static_cast<short *>(target))) == li);
		}
	};

	template<>
	struct Converter<unsigned short>{
		bool operator()(void * target, const std::string & s) const{
			unsigned long int uli;
			if(!Converter<unsigned long int>()(&uli, s)) return false;

			*(static_cast<unsigned short *>(target)) = static_cast<unsigned short>(uli);
			return(static_cast<unsigned long int>(*(static_cast<unsigned short *>(target))) == uli);
		}
	};

	template<>
	struct Converter<double>{
		bool operator()(void * target, const std::string & s) const{
			char * ptr;
			*(static_cast<double *>(target)) = std::strtod(s.c_str(), &ptr);
			return ptr == s.c_str() + s.length();
		}
	};

	template<>
	struct Converter<float>{
		bool operator()(void * target, const std::string & s) const{
			double d;
			if(!Converter<double>()(&d, s)) return false;

			if(d <= std::numeric_limits<float>::lowest() || d >= std::numeric_limits<float>::max()){
				return false;
			}
			*(static_cast<float *>(target)) = static_cast<float>(d);
			return true;
		}
	};

	struct Case{
		std::string name_long;
		char name_short;
		void * target;
		std::string type_name;
		converter_type conv;
		std::string description;
		std::string default_value;

		Case(const std::string & sw_long, char sw_short){
			if(sw_long.length() == 0){
				throw topion_definition_error("Empty switch is not allowed.");
			}else if(sw_long.length() == 1){
				if(sw_short != '\0'){
					throw topion_definition_error("Multiple short switch is not allowed. (In case the variable name is one-character, do NOT specify the short switch.)");
				}
				name_long = "";
				name_short = sw_long[0];
			}else{
				name_long = sw_long;
				for(size_t i = 0; i < name_long.length(); ++i){
					if(name_long[i] == '_') name_long[i] = '-';
				}
				name_short = sw_short;
			}
		}
	};

	using caselist_type = std::deque<Case>;
	using name2case_type = std::unordered_map<std::string, size_t>;
	using specification_type = std::unordered_map<size_t, const char *>;
	using mandatory_list_type = std::deque<size_t>;

	inline void register_name2case(caselist_type & top, name2case_type & tnc, const std::string & name_long, char name_short, size_t position){
		if(!(name_long.empty())){
			if(tnc.count(name_long) > 0){
				std::stringstream ss;
				ss << "Switch \"--" << name_long << "\" is specified twice.";
				throw topion_definition_error(ss.str());
			}
#ifdef TOPION_DEBUG
std::cerr << "Added long switch \"--" << name_long << "\"" << std::endl;
#endif
			tnc[name_long] = top.size() - 1;
		}

		if(name_short != '\0'){
			std::string name_short_str(1, name_short);
			if(tnc.count(name_short_str) > 0){
				std::stringstream ss;
				ss << "Switch \"-" << name_short_str << "\" is specified twice.";
				throw topion_definition_error(ss.str());
			}
			tnc[name_short_str] = top.size() - 1;
#ifdef TOPION_DEBUG
std::cerr << "Added short switch \"-" << name_short_str << "\"" << std::endl;
#endif
		}
	}

	template <class T>
	struct CaseAddition{
		CaseAddition(caselist_type & top, name2case_type & tnc, mandatory_list_type & ml, bool mandatory, const char * tname, T & target, converter_type converter, const std::string & sw_long, char sw_short, const std::string & desc){
			Case c(sw_long, sw_short);

			c.target = (void *)(&target);
			c.type_name = tname;
			c.conv = converter;
			c.description = desc;
			c.default_value = "";
			top.push_back(c);

			register_name2case(top, tnc, c.name_long, c.name_short, top.size() - 1);
			if(mandatory){
				ml.push_back(top.size() - 1);
			}
		}
	};

	template <>
	struct CaseAddition<bool>{
		CaseAddition(caselist_type & top, name2case_type & tnc, mandatory_list_type & ml, bool mandatory, const char * tname, bool & target, converter_type converter, const std::string & sw_long, char sw_short, const std::string & desc){
			Case c(sw_long, sw_short);

			c.target = (void *)(&target);
			c.type_name = "";
			c.conv = converter;
			c.description = desc;
			c.default_value = "";
			top.push_back(c);

			register_name2case(top, tnc, c.name_long, c.name_short, top.size() - 1);
			// "mandatory" flag should be rejected
			if(mandatory){
				std::stringstream ss;
				ss << "Switch \"" << sw_long << "\": Switch without an argument cannot be mandatory";
				throw topion_definition_error(ss.str());
			}
		}
	};

	struct StandaloneSetup{
		StandaloneSetup(size_t & curmin, size_t & curmax, std::string & curargname, size_t newmin, size_t newmax, const std::string & newargname){
			if(newmin > newmax){
				std::stringstream ss;
				ss << "Range of acceptable standalone parameters invalid: minimum = " << newmin << ", maximum = " << newmax;
				throw topion_definition_error(ss.str());
			}
			curmin = newmin;
			curmax = newmax;
			curargname = newargname;
		}
	};
} // TopionUtil

struct Topion{
protected:
	TopionUtil::caselist_type TOPION_CASES;
	TopionUtil::name2case_type TOPION_NAME2CASE;
	TopionUtil::mandatory_list_type TOPION_MANDATORY;
	std::deque<std::string> TOPION_STANDALONES;
	size_t TOPION_STANDALONE_MIN = 0;
	size_t TOPION_STANDALONE_MAX = 0;
	std::string TOPION_STANDALONE_ARGNAME;
public:
	template <class IO>
	void topion_usage(IO & out, size_t display_width, size_t tab_width){
		if(display_width <= tab_width){
			throw topion_definition_error("For 'usage', 'display_width' must be larger than 'tab_width'");
		}

		out << "Usage: [PROGRAMNAME] [OPTIONS]";
		if(!(TOPION_STANDALONE_ARGNAME.empty())){
			out << " " << TOPION_STANDALONE_ARGNAME;
		}
		out << std::endl;
		out << "[OPTIONS] are:" << std::endl;
		for(auto it = TOPION_CASES.begin(); it != TOPION_CASES.end(); ++it){
			std::stringstream head;
			if(it->name_short != '\0'){
				head << "-" << it->name_short;
			}
			if(!(it->name_long.empty())){
				if(it->name_short != '\0') head << ", ";
				head << "--" << it->name_long;
			}
			if(!(it->type_name.empty())){
				if(!(it->name_long.empty())) head << "=";
				head << "[" << it->type_name << "]";
			}
			head << ":";

			// Option name
			out << head.str();
			if(head.str().length() < tab_width){
				for(size_t i = head.str().length(); i < tab_width; ++i) out << ' ';
			}else{
				out << std::endl;
				for(size_t i = 0; i < tab_width; ++i) out << ' ';
			}

			// Description
			size_t displayed = 0;
			for(size_t p = 0; p < it->description.length(); ++p){
				if(displayed == display_width - tab_width || it->description[p] == '\n'){
					out << std::endl;
					for(size_t i = 0; i < tab_width; ++i) out << ' ';
					displayed = 0;
				}
				if(it->description[p] != '\n'){
					out << it->description[p];
					++displayed;
				}
			}
			out << std::endl;
		}
	}

	template <class IO>
	void topion_usage(IO & out){
		topion_usage(out, 70, 20);
	}

	void topion_parse(int argc, char ** argv){
		std::deque<std::string> errors;
		TopionUtil::specification_type specifications;

		// ------------------------------------------------------------
		// Parse the outline
		// ------------------------------------------------------------
		bool switch_ended = false;
		bool parse_ended_midway = false;
		for(int i = 1; i < argc; ++i){
			if(!switch_ended && argv[i][0] == '-'){
				std::string sw;
				char * direct_param = nullptr; // the parameter for the switch, without separated by spaces
				bool short_switch = false;
				if(argv[i][1] == '-'){
					// Long switch
					char * eql = std::strchr(&(argv[i][2]), '=');
					if(eql){
						sw.assign(&(argv[i][2]), eql - &(argv[i][2]));
						direct_param = eql + 1;
					}else{
						sw.assign(&(argv[i][2]));
					}

					if(sw.empty()){
						switch_ended = true;
						continue;
					}else if(sw.length() == 1){
						std::stringstream ss;
						ss << "Switch \"" << sw << "\": long switch must have two or more characters.";
						errors.push_back(ss.str());
						parse_ended_midway = true;
						break;
					}
				}else{
					// Short switch
					sw.assign(1, argv[i][1]);
					short_switch = true;
					if(argv[i][2] != '\0'){
						direct_param = &(argv[i][2]);
					}
				}

				auto f = TOPION_NAME2CASE.find(sw);
				if(f == TOPION_NAME2CASE.end()){
					std::stringstream ss;
					ss << "Switch \"" << sw << "\" is not defined.";
					errors.push_back(ss.str());
					parse_ended_midway = true;
					break;
				}
				size_t case_id = f->second;
				TopionUtil::Case & c = TOPION_CASES[case_id];

				if(c.type_name.empty()){
					if(direct_param){
						std::stringstream ss;
						ss << "Switch \"" << sw << "\" cannot receive a parameter but specified.";
						errors.push_back(ss.str());
					}
					*(static_cast<bool *>(c.target)) = true;
#ifdef TOPION_DEBUG
					std::cout << "SWITCH " << sw << " TRUE" << std::endl;
#endif // TOPION_DEBUG
				}else{
					// Where is the parameter?
					if(specifications.find(case_id) != specifications.end()){
						std::stringstream ss;
						ss << "Switch \"" << sw << "\" is specified twice or more.";
						errors.push_back(ss.str());
					}
					if(direct_param){
						specifications[case_id] = direct_param;
					}else{
						if(c.type_name.empty()){
							specifications[case_id] = "";
						}else{
							if(i == argc - 1){
								std::stringstream ss;
								ss << "Switch \"" << sw << "\" requires a parameter but not specified.";
								errors.push_back(ss.str());
								parse_ended_midway = true;
								break;
							}
							++i;
							specifications[case_id] = argv[i];
						}
					}

	#ifdef TOPION_DEBUG
					std::cout << "SWITCH " << sw << " VALUE " << specifications[case_id] << std::endl;
	#endif // TOPION_DEBUG
				}
			}else{
				TOPION_STANDALONES.emplace_back(argv[i]);
			}
		}

		if(!parse_ended_midway){
			// ------------------------------------------------------------
			// Check the number of standalone parameters
			// ------------------------------------------------------------
			if(TOPION_STANDALONES.size() < TOPION_STANDALONE_MIN || TOPION_STANDALONES.size() > TOPION_STANDALONE_MAX){
				std::stringstream ss;
				if(TOPION_STANDALONE_MAX == 0){
					ss << "Standalone parameters cannot be accepted";
				}else if(TOPION_STANDALONE_MIN == TOPION_STANDALONE_MAX){
					if(TOPION_STANDALONE_MIN == 1){
						ss << "Just 1 standalone parameter is required";
					}else{
						ss << "Just " << TOPION_STANDALONE_MIN << " standalone parameters are required";
					}
				}else if(TOPION_STANDALONE_MIN == 0){
					ss << "At most " << TOPION_STANDALONE_MAX << " standalone parameters can be accepted";
				}else if(TOPION_STANDALONE_MAX == std::numeric_limits<size_t>::max()){
					if(TOPION_STANDALONE_MIN == 1){
						ss << "At least 1 standalone parameter is required";
					}else{
						ss << "At least " << TOPION_STANDALONE_MIN << " standalone parameters are required";
					}
				}else{
					ss << "Number of standalone parameters must be between " << TOPION_STANDALONE_MIN << " and " << TOPION_STANDALONE_MAX << std::endl;
				}
				ss << " (specified " << TOPION_STANDALONES.size() << ").";
				errors.push_back(ss.str());
			}

			// ------------------------------------------------------------
			// Parse values
			// ------------------------------------------------------------
			for(auto it = specifications.begin(); it != specifications.end(); ++it){
				TopionUtil::Case & c = TOPION_CASES[it->first];
				if(!(c.conv(c.target, it->second))){
					std::stringstream ss;
					if(!(c.name_long.empty())){
						ss << "Switch \"--" << c.name_long << "\"";
					}else{
						ss << "Switch \"-" << c.name_short << "\"";
					}
					ss << ": Value \"" << it->second << "\" is invalid for this switch.";
					errors.push_back(ss.str());
				}
			}

			for(auto it = TOPION_MANDATORY.begin(); it != TOPION_MANDATORY.end(); ++it){
				if(specifications.find(*it) == specifications.end()){
					std::stringstream ss;
					TopionUtil::Case & c = TOPION_CASES[*it];
					if(!(c.name_long.empty())){
						ss << "Switch \"--" << c.name_long << "\"";
					}else{
						ss << "Switch \"-" << c.name_short << "\"";
					}
					ss << " is mandatory but not specified.";
					errors.push_back(ss.str());
				}
			}
		}

		// ------------------------------------------------------------
		// Error occurred?
		// ------------------------------------------------------------
		if(!(errors.empty())){
			std::stringstream ss;
			if(errors.size() == 1){
				ss << "An error found when parsing the command line: " << errors[0] << std::endl;
			}else{
				ss << errors.size() << " errors found when parsing the command line:" << std::endl;
				for(size_t i = 0; i < errors.size(); ++i){
					ss << "(" << (i+1) << "/" << errors.size() << ") " << errors[i] << std::endl;
				}
			}
			throw topion_parsing_error(ss.str());
		}
	}

	const std::deque<std::string> & topion_standalones() const{
		return TOPION_STANDALONES;
	}

	typename std::deque<std::string>::size_type topion_standalone_size() const{
		return TOPION_STANDALONES.size();
	}

	const std::string & operator[](typename std::deque<std::string>::size_type p) const{
		return TOPION_STANDALONES[p];
	}

	void topion_release_parser(){
		TOPION_CASES.clear();
		TOPION_NAME2CASE.clear();
		TOPION_MANDATORY.clear();
	}
};

#define TOPION_ADD_O(    T, var,                                   desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_OS(   T, var,             short,                desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_OA(   T, var,                    argname,       desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_OSA(  T, var,             short, argname,       desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_OC(   T, var,                             CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_OSC(  T, var,             short,          CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, CONV, #var, (short), (desc)}
#define TOPION_ADD_OAC(  T, var,                    argname, CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_OSAC( T, var,             short, argname, CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, CONV, #var, (short), (desc)}
#define TOPION_ADD_OD(   T, var, defaultval,                       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_ODS(  T, var, defaultval, short,                desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_ODA(  T, var, defaultval,        argname,       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_ODSA( T, var, defaultval, short, argname,       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_ODC(  T, var, defaultval,                 CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_ODSC( T, var, defaultval, short,          CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, #T,        var, CONV, #var, (short), (desc)}
#define TOPION_ADD_ODAC( T, var, defaultval,        argname, CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_ODSAC(T, var, defaultval, short, argname, CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, false, (argname), var, CONV, #var, (short), (desc)}
#define TOPION_ADD_M(    T, var,                                   desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_MS(   T, var,             short,                desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_MA(   T, var,                    argname,       desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_MSA(  T, var,             short, argname,       desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_MC(   T, var,                             CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_MSC(  T, var,             short,          CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, CONV, #var, (short), (desc)}
#define TOPION_ADD_MAC(  T, var,                    argname, CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_MSAC( T, var,             short, argname, CONV, desc) T var = {};           TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, CONV, #var, (short), (desc)}
#define TOPION_ADD_MD(   T, var, defaultval,                       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_MDS(  T, var, defaultval, short,                desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_MDA(  T, var, defaultval,        argname,       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, TopionUtil::Converter<T>(), #var, '\0', (desc)}
#define TOPION_ADD_MDSA( T, var, defaultval, short, argname,       desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, TopionUtil::Converter<T>(), #var, (short), (desc)}
#define TOPION_ADD_MDC(  T, var, defaultval,                 CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_MDSC( T, var, defaultval, short,          CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  #T,        var, CONV, #var, (short), (desc)}
#define TOPION_ADD_MDAC( T, var, defaultval,        argname, CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, CONV, #var, '\0', (desc)}
#define TOPION_ADD_MDSAC(T, var, defaultval, short, argname, CONV, desc) T var = (defaultval); TopionUtil::CaseAddition<T> TOPION_CASE_##var = {TOPION_CASES, TOPION_NAME2CASE, TOPION_MANDATORY, true,  (argname), var, CONV, #var, (short), (desc)}

#define TOPION_STANDALONE_ATLEAST(newmin, newargname) TopionUtil::StandaloneSetup TOPION_STANDALONE_SETUP = {TOPION_STANDALONE_MIN, TOPION_STANDALONE_MAX, TOPION_STANDALONE_ARGNAME, (newmin), std::numeric_limits<size_t>::max(), (newargname)};
#define TOPION_STANDALONE_ATMOST(newmax, newargname) TopionUtil::StandaloneSetup TOPION_STANDALONE_SETUP = {TOPION_STANDALONE_MIN, TOPION_STANDALONE_MAX, TOPION_STANDALONE_ARGNAME, 0, (newmax), (newargname)};
#define TOPION_STANDALONE_BETWEEN(newmin, newmax, newargname) TopionUtil::StandaloneSetup TOPION_STANDALONE_SETUP = {TOPION_STANDALONE_MIN, TOPION_STANDALONE_MAX, TOPION_STANDALONE_ARGNAME, (newmin), (newmax), (newargname)};
#define TOPION_STANDALONE_JUST(newnum, newargname) TopionUtil::StandaloneSetup TOPION_STANDALONE_SETUP = {TOPION_STANDALONE_MIN, TOPION_STANDALONE_MAX, TOPION_STANDALONE_ARGNAME, (newnum), (newnum), (newargname)};

#endif // TOPION_HPP_
