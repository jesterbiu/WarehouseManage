#include "warehouse_exception.hpp"
std::ostream& operator <<(std::ostream& os, const warehouse_exception& we)
{
	os << we.what() << " " << we.err_code() << "\n";
	return os;
}