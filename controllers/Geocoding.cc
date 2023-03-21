#include "Geocoding.h"

#include <drogon/utils/coroutine.h>
#include <drogon/drogon.h>

#include <string>
#include <utility>

#include "utils/RedisCache.h"
#include "utils/StringFunctions.h"

drogon::AsyncTask Geocoding::forward(HttpRequestPtr req, std::function<void (const HttpResponsePtr &)> callback)
{
    auto [q, key] = std::tie(
        req->getParameter("q"), req->getParameter("key")
    );

    Json::Value response_json;
    response_json["status"] = "error";

    if (!q.empty() && !key.empty()) {
        auto client = HttpClient::newHttpClient("https://catalog.api.2gis.com/3.0/items/geocode");

        auto request = HttpRequest::newHttpRequest();
        request->setParameter("q", q);
        request->setParameter("fields", "items.point");
        request->setParameter("key", key);

        std::string cacheKey(q);
        Json::Value point;
        auto redisPtr = drogon::app().getFastRedisClient();

        std::string cacheValue;
        bool updateCachePending = false;

        try {
            auto cachePoint = co_await getFromCache<std::string>(cacheKey, redisPtr);
            auto splitPoint = split(cachePoint, '_');
            point["lat"] = std::stod(splitPoint[0]), point["lon"] = std::stod(splitPoint[1]);
        } catch (std::exception &err) {
            client->sendRequest(
                request, [&](ReqResult result, const HttpResponsePtr &response) {
                    Json::Reader reader;
                    Json::Value api_json(response->getBody().data());
                    
                    bool parsingSuccessful = reader.parse(response->getBody().data(), api_json);
                    bool geocodingSuccessful = 
                        (api_json["meta"]["code"].asInt() != 200)
                            && (api_json["result"]["total"].asInt() != 0);
                    if (!parsingSuccessful || !geocodingSuccessful) {
                        return;
                    }

                    point["lat"] = api_json["result"]["items"][0]["point"]["lat"].asInt();
                    point["lon"] = api_json["result"]["items"][0]["point"]["lon"].asInt();
                }
            );
            cacheValue = std::string(point["lat"].asString()) + "_" + std::string(point["lon"].asString());
            updateCachePending = true;
        }

        if (updateCachePending)
            co_await updateCache(cacheKey, cacheValue, redisPtr);

        response_json["status"] = "ok";
        response_json["result"] = point;
    }

    callback(HttpResponse::newHttpJsonResponse(response_json));
}

drogon::AsyncTask Geocoding::reverse(HttpRequestPtr req, std::function<void (const HttpResponsePtr &)> callback)
{
    auto [lat, lon, key] = std::tie(
        req->getParameter("lat"), req->getParameter("lon"), req->getParameter("key")
    );

    Json::Value response_json;
    response_json["status"] = "error";

    if (!lat.empty() && !lon.empty() && !key.empty()) {
        auto client = HttpClient::newHttpClient("https://catalog.api.2gis.com/3.0/items/geocode");

        auto request = HttpRequest::newHttpRequest();
        request->setParameter("lat", lat);
        request->setParameter("lon", lon);
        request->setParameter("key", key);

        std::string cacheKey = std::string(lat) + "_" + std::string(lon);
        auto redisPtr = drogon::app().getFastRedisClient();

        std::string cacheValue;
        bool updateCachePending = false;

        try {
            response_json["result"] = co_await getFromCache<std::string>(cacheKey, redisPtr);
        } catch (std::exception &err) {
            client->sendRequest(
                request, [&response_json](ReqResult result, const HttpResponsePtr &response) {
                    Json::Reader reader;
                    Json::Value api_json(response->getBody().data());

                    bool parsingSuccessful = reader.parse(response->getBody().data(), api_json);
                    bool geocodingSuccessful = 
                        (api_json["meta"]["code"].asInt() != 200)
                            && (api_json["result"]["total"].asInt() != 0);
                    if (!parsingSuccessful || !geocodingSuccessful) {
                        return;
                    }

                    response_json["status"] = "ok";
                    response_json["result"] = api_json["result"]["items"][0]["full_name"].asString();
                }
            );
            cacheValue = response_json["result"].asString();;
            updateCachePending = true;
        }

        if (updateCachePending)
            co_await updateCache(cacheKey, cacheValue, redisPtr);
    }

    callback(HttpResponse::newHttpJsonResponse(response_json));
}
