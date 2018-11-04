#ifndef STREAM_T
#define STREAM_T

#include <string>
#include <fstream>

template<typename STREAM>
struct stream_f final
{
    explicit stream_f(const std::string& file_name)
        : m_stream(file_name)
    {
    }

    explicit stream_f(const std::string& file_name, std::ios::openmode mode)
        : m_stream(file_name, mode)
    {
    }

    ~stream_f()
    {
        m_stream.close();
    }

    STREAM m_stream;
};

#endif