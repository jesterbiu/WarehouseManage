#pragma once
#include <string>

namespace WarehouseManage
{
	using Time = std::time_t;
	using Job = int;
	struct Personnel
	{
		Personnel() {}
		Personnel(const std::string& id) :
			personnel_id(id) {}
		Personnel(const Personnel& oth) :
			personnel_id(oth.personnel_id) {}
		Personnel& operator =(const Personnel& rhs)
		{
			if (this != &rhs)
			{
				personnel_id = rhs.personnel_id;
			}
			return *this;
		}
		~Personnel() {}
		
		explicit operator bool() const
		{
			return personnel_id.size() > 0;
		}
		bool operator ==(const Personnel& oth) const
		{
			return personnel_id == oth.personnel_id;
		}
		
		std::string personnel_id;
		std::string password;
	};
}

