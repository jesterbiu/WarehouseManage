#pragma once
#include <stdexcept>

class warehouse_except : public std::exception
{
	const char* message;
	const int error_code;
public:
	warehouse_except(const char* msg, int errcode = -1) :
		message(msg), error_code(errcode) {}
	
};