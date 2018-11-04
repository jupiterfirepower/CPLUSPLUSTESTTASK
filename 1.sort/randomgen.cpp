#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include "streamtemplate.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    std::string outputfilename;
	
    if(argc <= 1)
    {
	std::cout << "Usage: [randomgen outputfilename] example: ./randomgen rndinputdata.txt" << std::endl;
	outputfilename = "rndinputdata.txt";
	std::cout << "Used defaultfilename in current directory: " << outputfilename << std::endl;
    }
    else
    {
	outputfilename = argv[1];
    }
    
    try
    {
	stream_f<std::ofstream> stream(outputfilename);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());

	std::fstream::pos_type size = 0;
	const auto max_file_size = 1024 * 1024 * 1024; //1GB

	while(size < max_file_size)
	{
	    std::ostringstream str;
	    str << dis(gen);
	    const auto text = str.str();
	    stream.m_stream << text << "\n";
	    size += (text.length() + 1);
	}
    }
    catch (const runtime_error& ex)
    {
	std::cerr << ex.what() << endl;
    }
    catch (exception& ex)
    {
	std::cerr << ex.what() << endl;
    }
    catch (...)
    {
	std::cerr << "error ..." << endl;
    }
    
    return 0;
}
