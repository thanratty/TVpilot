// vec-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>

class mydata
{
public:
    mydata(unsigned _num) : num(_num) {};
    unsigned getNum() const { return num; }
    unsigned operator()() const { return num; }
    unsigned operator!() const { return num * 2; }
private:
    unsigned num;
};



int main()
{
    std::string str("Hello World");
    std::cout << str << "\n";

    std::hash<std::string>  strhasher;
    auto ahash = strhasher(str);

    size_t strhasher2 = std::hash<std::string>{}(str);

    auto strhasher3 = std::hash<std::string>()(str);


    std::vector<mydata>  inst({1,2,3,4,5});

    std::cout << "size " << inst.size() << "\n";


    for (const mydata& elem : inst)
    {
        std::cout << elem.getNum() << "\n";
        std::cout << elem() << "\n";
        std::cout << !elem << "\n";
    }


    (void) getchar();

    return 0;

}
