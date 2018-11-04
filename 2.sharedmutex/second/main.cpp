#include <iostream>
#include <exception>
#include <list>
#include <mutex>
#include <sys/time.h>
#include <sys/resource.h>
#include "shared_mutex.hpp"
#include "shared_recursive_mutex.hpp"

int main()
{
    try
    {
	sm::shared_mutex sm;
        shared_recursive_mutex srm;
	
    } catch(const std::exception& e)
    {
	    std::cerr << "Exception: " << e.what() << std::endl;
    } catch(...)
    {
	    std::cerr << "Unknown exception" << std::endl;
    }
    return 0;
}