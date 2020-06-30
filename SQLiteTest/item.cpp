#include "item.hpp"
using namespace WarehouseManage;
std::ostream& WarehouseManage::operator<<(std::ostream& os, const Location& loc)
{
	return os << loc.shelf << loc.slot;
}
