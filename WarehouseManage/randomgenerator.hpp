#pragma once

#include <random>
#include "uncopiable.hpp"
typedef unsigned range;

class randomGenerator: private uncopiable
{
public:
	//constructor: default
	//destructor: default

	//get numbers
	unsigned int get_uniform() 
	{
		return uni_disb(engine); 
	}
	unsigned get_uniform(range r) 
	{
		return get_uniform((range)0, r);
	}
	unsigned get_uniform(range l, range h);
	void get_uniform(unsigned *a, int sz) 
	{
		get_uniform(a, sz, UINT_MAX); 
	}
	void get_uniform(unsigned *a, int sz, range r)
	{
		get_uniform(a, sz, range(0), r);
	}
	void get_uniform(unsigned *a, int sz, range l, range h);

private:
	std::default_random_engine engine;
	std::uniform_int_distribution<unsigned> uni_disb;
	void reset_param();
};