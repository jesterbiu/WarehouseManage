#include "randomgenerator.hpp"

unsigned randomGenerator::get_uniform(range l, range h)//range: [l, h)
{
	std::uniform_int_distribution<unsigned>::param_type p(l, h - 1);
	uni_disb.param(p);
	unsigned rdn = uni_disb(engine);
	reset_param();
	return rdn;
}

void randomGenerator::get_uniform(unsigned* a, int sz, range l, range h)
{
	std::uniform_int_distribution<unsigned> uni_disb(l, h - 1);
	for (int i = 0; i != sz; i++)
		a[i] = uni_disb(engine);
}

void randomGenerator::reset_param()
{
	std::uniform_int_distribution<unsigned>::param_type p(0, UINT_MAX - 1);
	uni_disb.param(p);
}