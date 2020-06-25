#pragma once
#include <string>

namespace WarehouseManage
{	
	enum class Personnel_Role { Invalid, Admin, Manager, Picker };
	inline int role_to_int(const Personnel_Role& role) 
	{
		switch (role)
		{
		case Personnel_Role::Admin:
			return 0;
		case Personnel_Role::Manager:
			return 1;
		case Personnel_Role::Picker:
			return 2;		
		default:
			throw warehouse_exception("Unknown role!");
			break;
		}
	}
	inline Personnel_Role int_to_role(int r)
	{
		switch (r)
		{
		case 0:
			return Personnel_Role::Admin;
		case 1:
			return Personnel_Role::Manager;
		case 2:
			return Personnel_Role::Picker;
		default:
			throw warehouse_exception("Unknown role!");
			break;
		}
	}

	class Personnel
	{
	public:
		Personnel() : role(Personnel_Role::Invalid) {}
		Personnel(Personnel_Role role, const std::string& id, const std::string& pw) :
			role(role), personnel_id(id), password(pw) 
		{}
		Personnel(const Personnel& oth) :
			role(oth.role), personnel_id(oth.personnel_id), password(oth.password) 
		{}
		Personnel& operator =(const Personnel& rhs)
		{
			if (this != &rhs)
			{
				role = rhs.role;
				personnel_id = rhs.personnel_id;
				password = rhs.password;
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
	
		Personnel_Role role;
		std::string personnel_id;
		std::string password;
	};
}

