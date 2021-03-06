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
#include <SQLiteCpp/SQLiteCpp.h>

using namespace std;

class SqliteClient
{
public: 
    SqliteClient(): _db("cache.db3", SQLite::OPEN_READWRITE)
    {
    }

    bool BeginTransaction()
    {
        _db.exec("BEGIN");
        return true;
    }

    bool CommitTransaction()
    {
        _db.exec("COMMIT");
        return true;
    }

    bool AbortTransaction()
    {
        _db.exec("ROLLBACK");
        return true;
    }

    bool AddOrUpdateInCache(std::string key, std::string value)
    {
        try
        {
            // Compile a SQL query, containing one parameter (index 1)
            SQLite::Statement query(_db, "UPDATE cache SET value=? WHERE key=?");
            
            // Bind the integer value 6 to the first parameter of the SQL query
            query.bind(1, value);
            query.bind(2, key);
            
            auto res = query.exec();
            std::cout << "res: " << res << std::endl;
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
            // Compile a SQL query, containing one parameter (index 1)
            SQLite::Statement query(_db, "DELETE FROM cache WHERE key=?");
            
            // Bind the integer value 6 to the first parameter of the SQL query
            query.bind(1, key);
            auto res = query.exec();
            std::cout << "res: " << res << std::endl;
            return res;
        }
        catch (std::exception& e)
        {
            std::cout << "DeleteByKey(std::string key) exception: " << e.what() << std::endl;
        }
        return 0;
    }
    
    std::string GetValueByKey(std::string key)
    {
        std::string result="";
        try
        {
            // Compile a SQL query, containing one parameter (index 1)
            SQLite::Statement query(_db, "SELECT key, value FROM cache WHERE key=?");
            
            // Bind the integer value 6 to the first parameter of the SQL query
            query.bind(1, key);
            
            // Loop to execute the query step by step, to get rows of result
            while (query.executeStep())
            {
                // Demonstrate how to get some typed column value
                const char* key     = query.getColumn(0);
                const char* value   = query.getColumn(1);
                std::cout << "row: " << key << ", " << value << ", " << std::endl;
                result = std::string(value);
            }
        }
        catch (std::exception& e)
        {
            std::cout << "GetValueByKey(std::string key) exception: " << e.what() << std::endl;
        }
        return result;
    }

    std::map<std::string, std::string> GetCacheData()
    {
        std::map<std::string, std::string> resultMap = std::map<std::string, std::string>();

        try
        {
            // Compile a SQL query, containing one parameter (index 1)
            SQLite::Statement query(_db, "SELECT key, value FROM cache");
            std::cout << "GetCacheData()" << std::endl;
            
            // Loop to execute the query step by step, to get rows of result
            while (query.executeStep())
            {
                // Demonstrate how to get some typed column value
                const char* key     = query.getColumn(0);
                const char* value   = query.getColumn(1);
                std::cout << "row: " << key << ", " << value << ", " << std::endl;
                resultMap.insert(std::make_pair(std::string(key),std::string(value)));
            }
        }
        catch (std::exception& e)
        {
            std::cout << "GetCacheData() exception: " << e.what() << std::endl;
        }

        return resultMap;
    }
    
private:
    SQLite::Database _db;
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
    SqliteClient db = SqliteClient();

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
            SqliteClient dbtmp = SqliteClient();

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
    // Using SQLITE_VERSION would require #include <sqlite3.h> which we want to avoid: use SQLite::VERSION if possible.
    //  std::cout << "SQlite3 version " << SQLITE_VERSION << std::endl;
    std::cout << "SQlite3 version " << SQLite::VERSION << " (" << SQLite::getLibVersion() << ")" << std::endl;
    std::cout << "SQliteC++ version " << SQLITECPP_VERSION << std::endl;

    ////////////////////////////////////////////////////////////////////////////
    // Simple batch queries example :
    try
    {
        // Open a database file in create/write mode
        SQLite::Database    db("cache.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        std::cout << "SQLite database file '" << db.getFilename().c_str() << "' opened successfully\n";

        // Create a new table with an explicit "id" column aliasing the underlying rowid
        db.exec("DROP TABLE IF EXISTS cache");
        db.exec("CREATE TABLE cache (id INTEGER PRIMARY KEY, key TEXT, value TEXT)");

        // first row
        int nb = db.exec("INSERT INTO cache VALUES (NULL, \"one\", \"first\")");
        std::cout << "INSERT INTO cache VALUES (NULL, \"test\")\", returned " << nb << std::endl;

        // second row
        nb = db.exec("INSERT INTO cache VALUES (NULL, \"two\", \"second\")");
        std::cout << "INSERT INTO test VALUES (NULL, \"second\")\", returned " << nb << std::endl;

        // second row
        nb = db.exec("INSERT INTO cache VALUES (NULL, \"three\", \"third\")");
        std::cout << "INSERT INTO cache VALUES (NULL, \"three\")\", returned " << nb << std::endl;

        nb = db.exec("INSERT INTO cache VALUES (NULL, \"four\", \"four\")");
        std::cout << "INSERT INTO cache VALUES (NULL, \"four\")\", returned " << nb << std::endl;

        // update the second row
        nb = db.exec("UPDATE cache SET value=\"second-updated\" WHERE id='2'");
        std::cout << "UPDATE cache SET value=\"second-updated\" WHERE id='2', returned " << nb << std::endl;

        // Check the results : expect two row of result
        SQLite::Statement   query(db, "SELECT * FROM cache");
        std::cout << "SELECT * FROM cache :\n";
        while (query.executeStep())
        {
            std::cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\")\n";
        }

        SqliteClient dbclient = SqliteClient();
        dbclient.GetCacheData();
        dbclient.DeleteByKey("one");
        std::cout << "After delete" << std::endl;
        // Check the results : expect two row of result
        SQLite::Statement   queryafter(db, "SELECT * FROM cache");
        while (queryafter.executeStep())
        {
            std::cout << "row (" << queryafter.getColumn(0) << ", \"" << queryafter.getColumn(1) << ", \"" << queryafter.getColumn(2)<< "\")\n";
        }
        dbclient.AddOrUpdateInCache("three","third updated");
        std::cout << "After Update" << std::endl;
        // Check the results : expect two row of result
        SQLite::Statement   queryafterupd(db, "SELECT * FROM cache");
        while (queryafterupd.executeStep())
        {
            std::cout << "row (" << queryafterupd.getColumn(0) << ", \"" << queryafterupd.getColumn(1) << ", \"" << queryafterupd.getColumn(2)<< "\")\n";
        }
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return EXIT_FAILURE; // unexpected error : exit the example program
    }
    remove("cache.db3");

    std::cout << "everything ok, quitting\n";

    return EXIT_SUCCESS;
}
