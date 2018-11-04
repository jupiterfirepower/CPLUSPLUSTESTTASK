#include <iostream>
#include "shared_mutex.hpp"

int main()
{
    try
    {

        recursive_shared_mutex srm;
        srm.lock();
        srm.lock();
        std::cout << "recursive_shared_mutex" << std::endl;
        srm.unlock();
        srm.unlock();
    } 
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    } 
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
    return 0;
}