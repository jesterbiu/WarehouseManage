#pragma once
#include <stdexcept>
#include <iostream>

class warehouse_exception : public std::exception
{
	const char* message;
	const int error_code;
public:
	warehouse_exception(const char* msg, int errcode = -1) :
		message(msg), error_code(errcode) 
	{}

	int err_code() const
	{ return error_code; }

	const char* what() const
	{
		return message;
	}	
};

std::ostream& operator <<(std::ostream& os, const warehouse_exception& we);