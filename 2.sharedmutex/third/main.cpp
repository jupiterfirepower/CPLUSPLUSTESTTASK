#include <iostream>
#include <chrono>
#include <thread>
#include "shared_mutex.hpp"

recursive_shared_mutex mutex;

void work()
{
    mutex.lock();
    std::cout << std::this_thread::get_id() << ": do work with the mutex" << std::endl;

    std::chrono::milliseconds sleepDuration(2500);
    std::this_thread::sleep_for(sleepDuration);

    mutex.unlock();
    std::cout << std::this_thread::get_id() << ": unlocked" << std::endl;
}

void Print(int a)
{
    mutex.lock();
    std::cout << a << " " << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(450));
	
    if(a <= 1)
    {
        std::cout << std::endl;
        mutex.unlock();
        return;
    }

    a--;
    Print(a);
    mutex.unlock();
}

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

        std::thread t1(work);
        std::thread t2(work);

        t1.join();
        t2.join();

        Print(9);

        std::thread p1(Print,9);
        std::thread p2(Print,9);

        p1.join();
        p2.join();
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