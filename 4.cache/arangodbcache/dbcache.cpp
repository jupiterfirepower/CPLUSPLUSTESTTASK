#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cassandra.h>
#include <exception>
#include <future>
#include <thread>
#include <stdexcept>
#include <queue>
#include <fuerte/fuerte.h>
#include <velocypack/Parser.h>
#include <velocypack/Iterator.h>
#include <velocypack/velocypack-aliases.h>
#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"

namespace f = ::arangodb::fuerte;
namespace fu = ::arangodb::fuerte;
using namespace arangodb::fuerte::v1;
using namespace arangodb::velocypack;

using namespace std;

class ArangoDbClient
{
public: 
    ArangoDbClient(std::string host, std::string username, std::string password) : 
        _eventLoopService(std::unique_ptr<f::EventLoopService>(new f::EventLoopService(1))), 
        _host(host), _username(username), _password(password)
    {
        
        try
        {
            auto conn = ConnectionBuilder().host("http://localhost:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user(username)
                                   .password(password)
                                   .connect(*_eventLoopService);

            auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
            auto result = conn->sendRequest(std::move(request));
            std::cout << "result->statusCode() - " << result->statusCode() << std::endl;

            if(result->statusCode() == f::StatusOK)
            {
                auto slice = result->slices().front();
                auto version = slice.get("version").copyString();
                auto server = slice.get("server").copyString();
                std::cout << "Connected to :  Server - " <<  server << " , Version - " << version << std::endl;
            }

            conn.reset();
        }
        catch(arangodb::velocypack::Exception& ex)
        {
            std::cerr <<  ex.what() << std::endl;
        }
        catch(std::exception& exp)
        {
            std::cerr <<  exp.what() << std::endl;
        }
    }

    bool BeginTransaction()
    {
        return true;
    }

    bool CommitTransaction()
    {
        return true;
    }

    bool AbortTransaction()
    {
        return true;
    }

    bool AddOrUpdateInCache(std::string key, std::string value)
    {
        try
        {
            auto connection = ConnectionBuilder().host(_host)
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user(_username)
                                   .password(_password)
                                   .connect(*_eventLoopService);

            auto request = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
            fu::VBuilder builder;
            builder.openObject();
            builder.add("query", fu::VValue("UPSERT { _key: '"+key+"' } INSERT { _key: '"+key+"', value: '"+value+"' } UPDATE { value: '"+value+"' } IN docs OPTIONS { waitForSync: true }")); 
            builder.close();

            request->addVPack(builder.slice());
            auto response = connection->sendRequest(std::move(request));
            std::cout << "response->statusCode() - " << response->statusCode() << std::endl;

            connection.reset();
        }
        catch (std::exception& e)
        {
            std::cout << "AddOrUpdateInCache exception: " << e.what() << std::endl;
        }

        return true;
    }
    
    int DeleteByKey(std::string key)
    {
        try
        {
            auto connection = ConnectionBuilder().host(_host)
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user(_username)
                                   .password(_password)
                                   .connect(*_eventLoopService);

            std::cout << "Connected. " << std::endl;

            auto request = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
            fu::VBuilder builder;
            builder.openObject();
            builder.add("query", fu::VValue("REMOVE { _key: '"+key+"' } IN docs OPTIONS { waitForSync: true }")); 
            builder.close();
            
            request->addVPack(builder.slice());
            auto response = connection->sendRequest(std::move(request));
            std::cout << "response->statusCode() - " << response->statusCode() << std::endl;

            connection.reset();
        }
        catch (std::exception& e)
        {
            std::cout << "DeleteByKey(std::string key) exception: " << e.what() << std::endl;
        }

        return 0;
    }
    
    std::string GetValueByKey(std::string key)
    {
        std::string resultVal="";

        try
        {
            auto connection = ConnectionBuilder().host(_host)
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user(_username)
                                   .password(_password)
                                   .connect(*_eventLoopService);
            std::cout << "Connected. " << std::endl;

            auto request = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
            fu::VBuilder builder;
            builder.openObject();
            builder.add("query", fu::VValue("FOR p IN docs LET a = p._key FILTER a == \""+key+"\" LIMIT 1 RETURN p"));     
            builder.close();
            request->addVPack(builder.slice());
            
            auto response = connection->sendRequest(std::move(request));
            std::cout << "sendRequest. " << std::endl;
            std::cout << "response->statusCode() - " << response->statusCode() << std::endl;
            
            if(response->statusCode() == f::StatusCreated)
            {
                auto slice = response->slices().front();

                std::cout << "slice.isObject() : " << slice.isObject() << std::endl;
                auto result = slice.get("result");
                std::cout << "result.isArray() : " << result.isArray() << std::endl;
                std::cout << "result.length() : " << result.length() << std::endl;
                std::cout << "result.toString() : " << result.toString() << std::endl;

                std::cout << "Iterating Array members:" << std::endl;
                for (auto const& it : ArrayIterator(result)) 
                {
                    auto key = it.get("_key").copyString();
                    std::cout << "Slice Key: " << key;
                    auto val = it.get("value").copyString();
                    std::cout << " Slice Value: " << val << std::endl;
                    resultVal = val;
                }
            }

            connection.reset();
        }
        catch (std::exception& e)
        {
            std::cout << "GetValueByKey(std::string key) exception: " << e.what() << std::endl;
        }

        return resultVal;
    }

    std::map<std::string, std::string> GetCacheData()
    {
        std::map<std::string, std::string> resultMap = std::map<std::string, std::string>();

        try
        {
            auto connection = ConnectionBuilder().host(_host)
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user(_username)
                                   .password(_password)
                                   .connect(*_eventLoopService);
            
            auto requestaql = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
            fu::VBuilder builder;
            builder.openObject();
            builder.add("query", fu::VValue("FOR x IN docs RETURN x")); 
            builder.close();
            
            requestaql->addVPack(builder.slice());
            auto response = connection->sendRequest(std::move(requestaql));
            std::cout << "load cache data response->statusCode() - " << response->statusCode() << std::endl;

            if(response->statusCode() == f::StatusCreated)
            {
                auto slice = response->slices().front();
                std::cout << "slice.isObject() : " << slice.isObject() << std::endl;
                auto result = slice.get("result");
                std::cout << "result.isArray() : " << result.isArray() << std::endl;
                std::cout << "result.length() : " << result.length() << std::endl;
                std::cout << "result.toString() : " << result.toString() << std::endl;

                std::cout << "Iterating Array members:" << std::endl;
                for (auto const& it : ArrayIterator(result)) 
                {
                    auto key = it.get("_key").copyString();
                    std::cout << "Slice Key: " << key;
                    auto val = it.get("value").copyString();
                    std::cout << " Value: " << val << std::endl;
                    resultMap.insert(std::make_pair(key,val));  
                }
            }
            connection.reset();
        }
        catch (std::exception& e)
        {
            std::cout << "GetCacheData() exception: " << e.what() << std::endl;
        }

        return resultMap;
    }
    
private:
    std::unique_ptr<f::EventLoopService> _eventLoopService;
    std::string _host;
    std::string _username;
    std::string _password;
};

struct i_db
{
    bool begin_transaction();
    bool commit_transaction();
    bool abort_transaction();
    std::string get(const std::string& key);
    std::string set(const std::string& key, const std::string& data);
    std::string remove(const std::string& key);
};

struct Cache: public i_db
{
public:
    Cache() : db(ArangoDbClient("vst://127.0.0.1:8529", "root", "dfvgbh123456789"))
    {
        handle = std::async(
                std::launch::async,
                &Cache::refreshCashe, this);

	    // load data from db 
        auto data = db.GetCacheData();

        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        for(const auto& v: data)
        {
            put(v.first, v.second);
            std::cout << "Cache() init map item: [ Key - " << v.first << ", Value - " << v.second << "]" << std::endl;
        }
    }

    ~Cache()
    {
        Dispose();
    }

    void Dispose()
    {
        cycle = false;
        handle.wait();
    }

    bool begin_transaction() 
    { 
        try
        {
            return db.BeginTransaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false; 
    }

    bool commit_transaction() 
    { 
        try
        {
            return db.CommitTransaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false;  
    }

    bool abort_transaction() 
    { 
        try
        {
            return db.AbortTransaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false;  
    }

    std::string get(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        if(exist(key))
        {
            auto val = _cache[key];
            if(isExpired(val.second))
            {
                std::string valdbdata = db.GetValueByKey(key); 
                put(key, valdbdata);
                return _cache[key].first;
            }
            return _cache[key].first;
        }

        return nullptr;
    }
    
    std::string set(const std::string& key, const std::string& data)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        if(begin_transaction())
        {
            // save to db code
            db.AddOrUpdateInCache(key, data);
            put(key, data);

            if(!commit_transaction())
            {
                abort_transaction();
            }
        }
        
        return _cache[key].first;
    }

    std::string remove(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        if(exist(key))
        {
            auto val = _cache[key];
            auto it = _cache.find(key);
            _cache.erase(it, _cache.end()); // erasing by range
            
            // remove from db 
            if(db.DeleteByKey(key))
            {
                std::cout << "delete by key - " << key << " ok." << std::endl;
            }

            return val.first;
        }

        return nullptr;
    }
private:
    volatile bool cycle = true;
    const int ttl = 30000; // 30 seconds
    std::recursive_mutex g_mutex;
    std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> _cache;
    std::future<void> handle;
    ArangoDbClient db;

    bool exist(const std::string& key)
    {
        return (_cache.count(key) > 0);
    }
    
    bool isExpired(const std::chrono::system_clock::time_point& date_time) 
    {
        return date_time + std::chrono::milliseconds(ttl) <= std::chrono::system_clock::now();
    }

    void put(const std::string& key, const std::string& data)
    {
        _cache[key] = std::make_pair(data,std::chrono::system_clock::now());
    }

    void refreshCashe() 
    {
        while (cycle) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15000));
            std::cout << "Refresh Cache()" << std::endl;
            std::lock_guard<std::recursive_mutex> lock(g_mutex);

            std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> _cachetmp;
            _cachetmp.insert(_cache.begin(), _cache.end());

            auto dbtmp = ArangoDbClient("vst://127.0.0.1:8529", "root", "dfvgbh123456789");

            for (auto& d: _cachetmp) 
            {
                auto time = d.second.second;
                if(isExpired(time))
                {
                    std::cout << "Refresh Cache() Expired for key - " << d.first << std::endl;
                    std::string valuedata = dbtmp.GetValueByKey(d.first); 
                    std::cout << "Refresh Cache() [ Key - " << d.first << ", Value - " << valuedata << "]" << std::endl;
                    put(d.first, valuedata);
                }
            }
        }
    }
};

int main ()
{
    ////////////////////////////////////////////////////////////////////////////
    // Simple batch queries example :
    try
    {
        auto dbclient = ArangoDbClient("vst://127.0.0.1:8529","root","dfvgbh123456789");
        dbclient.GetCacheData();
        dbclient.DeleteByKey("foo");
        std::cout << "After delete" << std::endl;
        dbclient.GetCacheData();
        dbclient.AddOrUpdateInCache("foo7","bar7");
        dbclient.AddOrUpdateInCache("foo5","bar55");
        std::cout << "After Update" << std::endl;
        dbclient.GetCacheData();
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return EXIT_FAILURE; // unexpected error : exit the example program
    }
 
    std::cout << "everything ok, quitting\n";

    return EXIT_SUCCESS;
}
