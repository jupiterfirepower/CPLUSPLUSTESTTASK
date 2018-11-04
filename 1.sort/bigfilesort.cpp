#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstdio>
#include "memusage.hpp"
#include "streamtemplate.hpp"

using namespace std;

template<typename T>
void printError(T& ex) noexcept(is_base_of<exception, T>::value)
{
    std::cerr << ex.what() << endl;
}

class sorted_file final
{
public:
    explicit sorted_file(std::ifstream& stream)
        : m_stream(stream), m_cur(0.0)
    {
    }

    bool has_data()
    {
        if(!m_buf.empty())
            return true;
        else
        {
            next();
            return (!m_buf.empty());
        }
    }

    double value() const
    {
        return m_cur;
    }

    void next()
    {
        m_buf.clear();
        std::getline(m_stream, m_buf);

        if(!m_buf.empty())
        {
            std::istringstream str(m_buf);
            str >> m_cur;
        }
    }

private:
    std::ifstream& m_stream;
    std::string m_buf;
    double m_cur;
};

class sorted_splitted_data final
{
public:
    explicit sorted_splitted_data(std::vector<sorted_file>& files)
        : m_files(files)
    {
    }

    bool has_data()
    {
        return std::any_of( m_files.begin(), m_files.end(),
            [] ( sorted_file& file ) { return file.has_data(); } );
    }

    double min()
    {
        std::vector<std::pair<double, std::size_t>> values;
        std::size_t count = 0;

        std::for_each( m_files.begin(), m_files.end(),
            [&] (sorted_file& file)
            {
                if( file.has_data() )
                    values.push_back(std::make_pair(file.value(), count));

                ++count;
            } );

        if( !values.empty() )
        {
            double min_value = values.front().first;
            std::size_t idx = values.front().second;

            for( const auto& p : values )
            {
                if( p.first < min_value )
                {
                    min_value = p.first;
                    idx = p.second;
                }
            }

            m_files[idx].next();
            return min_value;
        }
        else
            return 0.0;
    }

private:
    std::vector<sorted_file>& m_files;
};

ssize_t split_file(std::ifstream& stream)
{
    const std::size_t max_size = 1024 * 1024 * 100 / sizeof(double);

    ssize_t count = 0;

    while( stream )
    {
        std::vector<double> data;
        data.reserve(max_size);

        while( data.size() <= max_size && stream )
        {
            std::string text;
            std::getline(stream, text);

            if( !text.empty() )
            {
                std::istringstream str(text);
                double value = 0.0;
                str >> value;

                data.push_back(value);
            }
        }

        using std::cout;
        using std::endl;

        double vm, rss;
        process_mem_usage(vm, rss);
        cout << "VM: " << vm << " KB" << "; RSS: " << rss << " KB" << endl;

        if( !data.empty() )
        {
            std::sort(data.begin(), data.end());
            stream_f<std::ofstream> out(std::to_string(count) + ".tmp", std::ios::out | std::ios::trunc);

            if( out.m_stream )
            {
                for( const auto& v : data )
                    out.m_stream << v << "\n";
            }
            else
                return -1;

            ++count;
        }
    }

    return count;
}

int main(int argc, char *argv[])
{
    std::string inputfilename;
    std::string outputfilename;

    if(argc < 3)
    {
        std::cout << "Usage: ./bigfilesort inputdatafilename outputfiledataname" << std::endl;
        return 0;
    }
    else
    {
        inputfilename = argv[1];
        outputfilename = argv[2];
    }

    try
    {
        const auto start = std::chrono::high_resolution_clock::now();
        ssize_t count = 0;

        stream_f<std::ifstream> in(inputfilename, std::ios::in);

        if( in.m_stream )
        {
            count = split_file(in.m_stream);

            if( count < 0 )
            {
                std::cerr << "Couldn't split input file." << std::endl;
                return 1;
            }
        } 
        else
        {
            std::cerr << "Couldn't open input file." << std::endl;
            return 1;
        }

        std::vector<std::unique_ptr<stream_f<std::ifstream>>> in_streams;
        std::vector<sorted_file> in_files;

        for( ssize_t i = 0; i < count; ++i )
        {
            in_streams.push_back(std::make_unique<stream_f<std::ifstream>>(std::to_string(i) + ".tmp", std::ios::in));

            if( !in_streams.back()->m_stream )
            {
                std::cerr << "Couldn't open splitted files." << std::endl;

                return 1;
            }

            in_files.push_back(sorted_file(in_streams.back()->m_stream));
        }

        sorted_splitted_data data(in_files);
        stream_f<std::ofstream> out(outputfilename, std::ios::out | std::ios::trunc);

        if(!out.m_stream)
        {
            std::cerr << "Couldn't open output file." << std::endl;
            return 1;
        }

        while( data.has_data() )
            out.m_stream << data.min() << endl;

        const auto finish = std::chrono::high_resolution_clock::now();
        auto duration = finish - start;
        const auto min = std::chrono::duration_cast<std::chrono::minutes>(duration);
        duration -= min;
        const auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= sec;
        const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

        for( ssize_t i = 0; i < count; ++i )	
        {
            remove((std::to_string(i) + ".tmp").c_str()); // delete file
        }

        std::cout << "Elapsed time: "
                  << min.count() << " m "
                  << sec.count() << "."
                  << std::setw(3) << std::setfill('0') << milli.count() << " s" << std::endl;

    }
    catch (const runtime_error& ex)
    {
        printError(ex);
    }
    catch (exception& ex)
    {
        printError(ex);
    }
    catch (...)
    {
        std::cerr << "error ..." << endl;
    }

    return 0;
}
