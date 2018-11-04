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

using namespace std;

template<typename T>
void printError(T& ex) noexcept(is_base_of<exception, T>::value)
{
    std::cerr << ex.what() << endl;
}

void handle_eptr(std::exception_ptr eptr) // passing by value is ok
{
    try 
    {
        if (eptr) 
        {
            std::rethrow_exception(eptr);
        }
    } 
    catch(const std::exception& e) 
    {
        std::cerr << "Caught exception \"" << e.what() << std::endl;
    }
}

struct CassandraException : public std::exception
{
    CassandraException(std::string err)
    : _err(err)
    {}

    const char* what() const noexcept {
        return _err.c_str();
    }

private:
    std::string _err;
};

class Cassandra
{
public: 
    void Connect(std::string hosts)
    {
        _cluster = cass_cluster_new();
        _session = cass_session_new();
        cass_cluster_set_contact_points(_cluster, hosts.c_str());
        
        _connect_future = cass_session_connect(_session, _cluster);

        if (cass_future_error_code(_connect_future) != CASS_OK)
        {
            const char* message;
            size_t message_length;
            cass_future_error_message(_connect_future, &message, &message_length);
            std::cerr << "Unable to connect: " << (int)message_length << message << std::endl;
            //throw CassandraException(std::string(message));
        }
        else
        {
            std::cout << "Connected OK." << std::endl;
        }
    }

    bool doQuery(std::string query, int nparam, std::function<void(CassStatement*)> bindparams)
    {
        bool result = false;

        if(_cluster == nullptr)
        {
            Connect("127.0.0.1");
        }

        CassError rc = CASS_OK;
        CassStatement* statement = NULL;
        CassFuture* future = NULL;

        statement = cass_statement_new(query.c_str(), nparam);
        bindparams(statement);

        std::cout << "Statement prepeared OK." << std::endl;
        future = cass_session_execute(_session, statement);
        cass_future_wait(future);

        rc = cass_future_error_code(future);

        if (rc != CASS_OK) 
        {
            HandleError(query);
        } 
        else 
        {
            std::cout << query.substr(0, query.find(" ")) << " query OK." << std::endl;
            result = true;
        }

        cass_future_free(future);
        cass_statement_free(statement);

        return result;
    }

    bool AddOrUpdateInCache(std::string key, std::string value)
    {
        // store a free function
        std::function<void(CassStatement*)> insert_statement_prepared = [&] (CassStatement* statement)
            {
                CassUuid create_ts;
                CassUuidGen* uuid_gen = cass_uuid_gen_new();
                cass_uuid_gen_time(uuid_gen, &create_ts);
                cass_statement_bind_uuid(statement, 0, create_ts);
                cass_statement_bind_string(statement, 1, key.c_str());
                cass_statement_bind_string(statement, 2, value.c_str());
            };

        std::function<void(CassStatement*)> update_statement_prepared = [&] (CassStatement* statement)
            {
                auto uuid = GetIdByKey(key);
                cass_statement_bind_string(statement, 0, value.c_str());
                cass_statement_bind_uuid(statement, 1, uuid);
            };

        return GetValueByKey(key).empty() ? 
            doQuery("INSERT INTO testtask.cache(id, key,value) VALUES(?,?,?);", 3, [&] (CassStatement* statement) { insert_statement_prepared(statement); }) :
            doQuery("UPDATE testtask.cache SET value=? WHERE id=?;", 2, [&] (CassStatement* statement) { update_statement_prepared(statement); });
        
    }
    
    bool DeleteByKey(std::string key)
    {
        std::function<void(CassStatement*)> delete_statement_prepared = [&] (CassStatement* statement)
            {
                cass_statement_bind_uuid(statement, 0, GetIdByKey(key));
            };

        return doQuery("DELETE FROM testtask.cache WHERE id=?;", 1, [&] (CassStatement* statement) { delete_statement_prepared(statement); }); 
    }

    void GetByKey(std::string query, std::string key, std::function<void(const CassRow*)> row_getresult) 
    { 
        if(_cluster == nullptr)
        {
            Connect("127.0.0.1");
        }

        CassError rc = CASS_OK;
        CassStatement* statement = NULL;
        CassFuture* future = NULL;

        statement = cass_statement_new(query.c_str(), 1);
        cass_statement_bind_string(statement, 0, key.c_str());

        std::cout << "Statement prepeared OK." << std::endl;

        future = cass_session_execute(_session, statement);
        cass_future_wait(future);

        rc = cass_future_error_code(future);

        if (rc != CASS_OK) 
        {
            HandleError(query);
        } 
        else 
        {
            std::cout << "Statement query OK." << std::endl;
            const CassResult* result = cass_future_get_result(future);
            const CassRow* row = cass_result_first_row(result);

            if(row)
            {
                row_getresult(row);
            }

            cass_result_free(result);
        }

        cass_future_free(future);
        cass_statement_free(statement);
    }

    CassUuid GetIdByKey(std::string key)
    {
        CassUuidGen* uuid_gen = cass_uuid_gen_new();
        CassUuid uuid;
        cass_uuid_gen_time(uuid_gen, &uuid);

        std::function<void(const CassRow*)> row_getresult = [&] (const CassRow* row)
            {
                cass_value_get_uuid(cass_row_get_column(row, 0), &uuid);
            };

        GetByKey("SELECT id FROM testtask.cache WHERE key=? ALLOW FILTERING;", key, [&] (const CassRow* row) { row_getresult(row); }); 
        
        return uuid;
    }
    
    std::string GetValueByKey(std::string key)
    {
        std::string resultValue = "";

        std::function<void(const CassRow*)> row_getresult = [&] (const CassRow* row)
            {
                /* Now we can retrieve the column values from the row */
                const char* value;
                size_t value_length;
                /* Get the column value of "key" by name */
                cass_value_get_string(cass_row_get_column_by_name(row, "value"), &value, &value_length);
                resultValue = std::string(value);
            };

        GetByKey("SELECT value FROM testtask.cache WHERE key=? ALLOW FILTERING;", key, [&] (const CassRow* row) { row_getresult(row); }); 

        return resultValue;
    }

    std::map<std::string, std::string> GetCacheData()
    {
        std::map<std::string, std::string> resultMap = std::map<std::string, std::string>();

        if(_cluster == nullptr)
        {
            Connect("127.0.0.1");
        }

        CassError rc = CASS_OK;
        CassStatement* statement = nullptr;
        CassFuture* future = nullptr;
        const char* query = "SELECT key,value FROM testtask.cache;";

        statement = cass_statement_new(query, 0);

        future = cass_session_execute(_session, statement);
        cass_future_wait(future);

        rc = cass_future_error_code(future);

        if (rc != CASS_OK) 
        {
            HandleError(std::string(query));
        } 
        else 
        {
            std::cout << "Statement query OK." << std::endl;
            const CassResult* result = cass_future_get_result(future);
            CassIterator* iterator = cass_iterator_from_result(result);

            while (cass_iterator_next(iterator)) 
            {
                const CassRow* row = cass_iterator_get_row(iterator);

                auto keyColumnValue = cass_row_get_column(row, 0);
                                            
                const char* key_string_value;
                const char* string_value;
                size_t string_value_length;
                cass_value_get_string(keyColumnValue, &key_string_value, &string_value_length);

                auto ValColumnValue = cass_row_get_column(row, 1);
                cass_value_get_string(ValColumnValue, &string_value, &string_value_length);

                resultMap.insert(std::make_pair(key_string_value,string_value));

                std::cout << "item: Key - " << key_string_value << " Value - " << string_value << std::endl;
            }
            cass_result_free(result);
            cass_iterator_free(iterator);
        }

        cass_future_free(future);
        cass_statement_free(statement);

        return resultMap;
    }
    ~Cassandra() 
    {
        try
        {
            std::cout << "~Cassandra()" << std::endl;
            if(_connect_future != nullptr)
                cass_future_free(_connect_future);
            if(_cluster != nullptr)
                cass_cluster_free(_cluster);
            if(_session != nullptr)
                cass_session_free(_session);
        }
        catch (exception& ex)
        {
            printError(ex);
        }
        catch (...)
        {
            std::cerr << "error ..." << endl;
            handle_eptr(std::current_exception());
        }
    }
private:
    CassCluster* _cluster = nullptr;
    CassSession* _session = nullptr;
    CassFuture* _connect_future = nullptr;

    void HandleError(std::string query)
    {
        /* Handle error */
        const char* message;
        size_t message_length;
        cass_future_error_message(_connect_future, &message, &message_length);
        std::cerr << "Unable to run query: " << query << " Error message: " << message << std::endl;
    }
};

// Cache-entry
struct cacheItem
{
    cacheItem() :
        m_value(""), m_expiration_time(std::chrono::system_clock::now()) {}
    cacheItem(const std::string& value) :
        m_value(value), m_expiration_time(std::chrono::system_clock::now()) {}
    cacheItem(const cacheItem& value) :
        m_value(value.m_value), m_expiration_time(value.m_expiration_time) {}
    cacheItem(const cacheItem&& value) :
        m_value(value.m_value), m_expiration_time(value.m_expiration_time) {}

    cacheItem& operator=(const cacheItem& other)
    {
        if (this != &other) // self-assignment check expected
        { 	
            this->m_value = other.m_value;
            this->m_expiration_time = other.m_expiration_time;
        }
        return *this;
    }

    std::string m_value;
    std::chrono::system_clock::time_point m_expiration_time;
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
    Cache()
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

    /* No need transaction model for single insert/update or delete query - { return true; } */
    bool begin_transaction() { return true; }
    bool commit_transaction() { return true; }
    bool abort_transaction() { return true; }

    std::string get(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        if(exist(key))
        {
            auto val = _cache[key];
            if(isExpired(val.m_expiration_time))
            {
                std::string valdbdata = db.GetValueByKey(key); 
                put(key, valdbdata);
                return _cache[key].m_value;
            }
            return _cache[key].m_value;
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
        
        return _cache[key].m_value;
    }

    std::string remove(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        if(exist(key))
        {
            auto val = _cache[key];
            auto it = _cache.find(key);
            _cache.erase(it, _cache.end());    // erasing by range
            
            if(begin_transaction())
            {
                // remove from db 
                if(db.DeleteByKey(key))
                {
                   std::cout << "delete by key - " << key << " ok." << std::endl;
                   if(!commit_transaction())
                   {
                     abort_transaction();
                   }
                }
                
            }
                       
            return val.m_value;
        }

        return nullptr;
    }
private:
    const int ttl = 30000; // 30 seconds
    std::recursive_mutex g_mutex;
    std::unordered_map<std::string, cacheItem> _cache;
    std::future<void> handle;
    Cassandra db = Cassandra();
    volatile bool cycle = true;

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
        _cache[key] = cacheItem(data);
    }

    void refreshCashe() 
    {
        while (cycle) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15000));
            std::cout << "Refresh Cache()" << std::endl;
            std::lock_guard<std::recursive_mutex> lock(g_mutex);

            std::unordered_map<std::string, cacheItem> _cachetmp;
            _cachetmp.insert(_cache.begin(), _cache.end());
            Cassandra dbtmp = Cassandra();

            for (auto& d: _cachetmp) 
            {
                if(isExpired(d.second.m_expiration_time))
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

int main()
{
    auto cache = Cache();

    auto db = Cassandra();
    auto result = db.GetValueByKey("one");
    std::cout << "Value By Key - " << result << std::endl;
    std::cout << "DeleteByKey - " << db.DeleteByKey("two") << std::endl;
    std::cout << "AddOrUpdateInCache - " << db.AddOrUpdateInCache("two", "second1") << std::endl;
    std::cout << "AddOrUpdateInCache - " << db.AddOrUpdateInCache("two1", "second0") << std::endl;
    std::cout << "AddOrUpdateInCache - " << db.AddOrUpdateInCache("two12", "second12") << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100000));
    cache.Dispose();

    return 0;
}