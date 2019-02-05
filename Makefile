CC=g++
CCFLAGS=-std=c++11
EXE=topion_sample topion_sample_mandatory topion_sample_optional

default: $(EXE)

topion_sample: topion_sample.cpp
	$(CC) $< -o $@

topion_sample_mandatory: topion_sample_mandatory.cpp
	$(CC) $< -o $@

topion_sample_optional: topion_sample_optional.cpp
	$(CC) $< -o $@

topion_sample.cpp: topion.hpp
topion_sample_mandatory.cpp: topion.hpp
topion_sample_optional.cpp: topion.hpp

clean:
	rm -vf $(EXE)