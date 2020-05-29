#pragma once

class uncopiable
{
protected:
	uncopiable() {};
	~uncopiable() {};
private:
	uncopiable(const uncopiable& oth);
	uncopiable& operator=(const uncopiable& rhs);
};
