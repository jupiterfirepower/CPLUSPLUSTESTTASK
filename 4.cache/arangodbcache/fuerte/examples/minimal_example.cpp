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

int main()
{
    

    auto eventLoopService = std::unique_ptr<f::EventLoopService>(new f::EventLoopService(1));
    auto conn = ConnectionBuilder().host("http://localhost:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);
    try
    {
        auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
        std::cout << "createRequest" << std::endl;
        auto result = conn->sendRequest(std::move(request));
        std::cout << "sendRequest" << std::endl;
        std::cout << "result->statusCode() - " << result->statusCode() << std::endl;

        if(result->statusCode() == f::StatusOK)
        {
            auto slice = result->slices().front();
            auto version = slice.get("version").copyString();
            auto server = slice.get("server").copyString();
            std::cout << "Connected to :  Server - " <<  server << " , Version - " << version << std::endl;
        }

        f::WaitGroup wg;
        auto request1 = fu::createRequest(fu::RestVerb::Get, "/_api/version");
        auto cb = [&](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res) 
        {
            f::WaitGroupDone done(wg);

            if (error) 
            {
                std::cerr << "Error - " << fu::to_string(fu::intToError(error));
            } 
            else 
            {
                std::cout << "result->statusCode() - " << result->statusCode() << std::endl;
                if(res->statusCode() == f::StatusOK)
                {
                    auto slice = res->slices().front();
                    auto version = slice.get("version").copyString();
                    auto server = slice.get("server").copyString();
                    std::cout << "Connected to :  Server - " <<  server << " , Version - " << version << std::endl;
                }
            }
        };
        wg.add();
        conn->sendRequest(std::move(request1), cb);
        wg.wait();
        //conn.reset();

        /*std::cout << "New Connection " << std::endl;
        auto connection = ConnectionBuilder().host("vst://127.0.0.1:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);
        std::cout << "Connected. " << std::endl;
        VPackBuilder builder;
        builder.openObject();
        builder.add("collection" , VPackValue("docs"));
        builder.close();*/

        StringMap map = { { "collection", "docs" },  {"batchSize" , "3"} };
        auto requestv = f::createRequest(f::RestVerb::Put, "/_api/simple/all", map);
        //request->addVPack(builder.slice());
        auto resultv = conn->sendRequest(std::move(requestv));
        std::cout << "sendRequest Post " << std::endl;
        std::cout << "resultv->statusCode() - " << resultv->statusCode() << std::endl;
        //std::string key = resultv->slices().front().get("_key").copyString();
        //std::cout << "key:  " << key << std::endl;
        auto slices = result->slices();
        for(auto& slice : slices)
        {
            std::cout << "get 1" <<  std::endl;
            //auto name = slice.get("_key").copyString();
            //std::cout << "VSlice _key: " << name << std::endl;
            std::cout << "get 2" <<  std::endl;
            //auto slvalue = slice.get("value").copyString();
            std::cout << "VSlice : " << slice << std::endl;
        }    
 
       /* requestv = f::createRequest(f::RestVerb::Get, "/_api/document/documents/" + key);
        resultv = connection->sendRequest(std::move(requestv));
        std::cout << resultv->slices().front().get("foo1").copyString();*/


        /*auto requestCol = fu::createRequest(fu::RestVerb::Get, "/_api/simple/all");
        std::cout << "createRequest" << std::endl;
        result = connection->sendRequest(std::move(requestCol));
        std::cout << "sendRequest" << std::endl;
        std::cout << "result->statusCode() - " << result->statusCode() << std::endl;

        if(result->statusCode() == f::StatusOK)
        {
            std::cout << "for(auto& slice : result->slices())" <<  std::endl;
            auto slices = result->slices();
            for(auto& slice : slices)
            {
                std::cout << "get 1" <<  std::endl;
                auto name = slice.get("name").copyString();
                std::cout << "VSlice Name: " << name << std::endl;
                std::cout << "get 2" <<  std::endl;
                //auto slvalue = slice.get("value").copyString();
                std::cout << "VSlice : " << slice << std::endl;
            }
        }*/

        auto connection = ConnectionBuilder().host("vst://127.0.0.1:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);

        std::cout << "Connected. " << std::endl;

        auto requestaql = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
        fu::VBuilder builder;
        builder.openObject();
        builder.add("query", fu::VValue("FOR x IN docs RETURN x")); //    FOR p IN docs LET a = p._key FILTER a == \"foo\" LIMIT 1 RETURN p
        builder.close();
        requestaql->addVPack(builder.slice());
        auto response = connection->sendRequest(std::move(requestaql));
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
                auto keyv = it.get("_key").copyString();
                std::cout << "Slice Key: " << keyv;
                auto val = it.get("value").copyString();
                 std::cout << " Slice Value: " << val << std::endl;
            }
        }
        connection.reset();

        auto connectionu = ConnectionBuilder().host("vst://127.0.0.1:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);

        std::cout << "Connected. " << std::endl;

        auto requestaqlupd = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
        fu::VBuilder builderupd;
        builderupd.openObject();
        builderupd.add("query", fu::VValue("FOR d IN docs FILTER d._key == \"foo\" UPDATE d WITH { value: 'barupd' } IN docs OPTIONS { waitForSync: true } RETURN d")); 
        builderupd.close();
        requestaqlupd->addVPack(builderupd.slice());
        auto responseupd = connectionu->sendRequest(std::move(requestaqlupd));
        std::cout << "sendRequest. " << std::endl;
        std::cout << "response->statusCode() - " << responseupd->statusCode() << std::endl;
        connectionu.reset();



        auto connectionins = ConnectionBuilder().host("vst://127.0.0.1:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);

        std::cout << "Connected. " << std::endl;

        auto requestaqlins = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
        fu::VBuilder builderins;
        builderins.openObject();
        builderins.add("query", fu::VValue("INSERT { _key: \"foo6\", value: \"bar6\" } INTO docs OPTIONS { waitForSync: true }")); 
        builderins.close();
        requestaqlins->addVPack(builderins.slice());
        auto responseins = connectionins->sendRequest(std::move(requestaqlins));
        std::cout << "sendRequest. " << std::endl;
        std::cout << "response->statusCode() - " << responseins->statusCode() << std::endl;
        connectionins.reset();

///
        auto connectioninsu = ConnectionBuilder().host("vst://127.0.0.1:8529")
                                   .authenticationType(f::AuthenticationType::Basic)
                                   .user("root")
                                   .password("dfvgbh123456789")
                                   .connect(*eventLoopService);

        std::cout << "Connected. " << std::endl;

        auto requestaqlinsu = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
        fu::VBuilder builderinsu;
        builderinsu.openObject();
        builderinsu.add("query", fu::VValue("UPSERT { _key: 'foo' } INSERT { _key: 'key', value: 'key1' } UPDATE { value: 'key1' } IN docs OPTIONS { waitForSync: true }")); 
        builderinsu.close();
        requestaqlinsu->addVPack(builderinsu.slice());
        auto responseinsu = connectioninsu->sendRequest(std::move(requestaqlinsu));
        std::cout << "sendRequest. " << std::endl;
        std::cout << "response->statusCode() - " << responseinsu->statusCode() << std::endl;
        connectioninsu.reset();
        
        
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
