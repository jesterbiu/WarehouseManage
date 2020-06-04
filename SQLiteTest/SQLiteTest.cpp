#include "manage.hpp"
void simple_printf(const char* fmt...)
{
    // Initialize va
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        if (*fmt == 'd') {
            // Access next va starts from pos = -1
            int i = va_arg(args, int);
            std::cout << i << '\n';
        }
        else if (*fmt == 'c') {
            // note automatic conversion to integral type
            int c = va_arg(args, int);
            std::cout << static_cast<char>(c) << '\n';
        }
        else if (*fmt == 'f') {
            double d = va_arg(args, double);
            std::cout << d << '\n';
        }
        ++fmt;
    }

    va_end(args);
}

int main()
{
    manage m;
    auto d1 = data{ "001", "A1", 100 };
    m.add(d1);    
    auto result = m.find(d1.ID);
    if (std::get<bool>(result))
    {
        auto d2 = std::get<data>(result);
        std::cout << d2.ID << "\t" 
            << d2.loc << "\t" 
            << d2.stocks << "\n";
    }
    else
    {
        std::cout << "failed\n";
    }
}

