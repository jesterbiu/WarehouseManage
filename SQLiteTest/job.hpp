#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <type_traits>

// item_id, quantity, location
using good = std::tuple<int, int, std::string>;
// item_id, location, expected count, actual count
using inventory = std::tuple<int, std::string, int, int>;


struct job
{
	virtual void perform() = 0;
	virtual ~job() {}
};

struct select : public job
{
	select(const good& g) : g(g) {}
	void perform() override
	{
		std::cout << "select:\n"
			<< "ID\tCount\tLocation\n"
			<< std::get<0>(g) << "\t"
			<< std::get<1>(g) << "\t"
			<< std::get<2>(g) << "\n";
		std::cout << "press key to confirm:";
		system("pause");
		system("cls");
	}
	~select() {}
private:
	good g;
};

struct stock : public job
{
	stock(const inventory& in) : in(in) {}
	void perform() override
	{
		std::cout << "input actual stock:\n"
			<< "ID\tLoc\tExp\tActual\n"
			<< std::get<0>(in) << "\t"
			<< std::get<1>(in) << "\t"
			<< std::get<2>(in) << "\n";
		std::cin >> std::get<3>(in);
		system("cls");
	}
	~stock() {}
private:
	inventory in;
};

struct person
{
	person(job* j) : j(j) {}
	void do_job()
	{
		j->perform();
	}
private:
	job* j;
};

template<typename JobType, typename Arg>
std::unique_ptr<job> generate_job(Arg& arg)
{
	static_assert(std::is_base_of<job, JobType>::value, "type parameter of this class must derive from job");
	return std::make_unique<JobType>(arg);
}