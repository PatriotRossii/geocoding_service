#include "Geocoding.h"

#include <utility>

#include <drogon/drogon.h>

void Geocoding::forward(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
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
    }

    callback(HttpResponse::newHttpJsonResponse(response_json));
}

void Geocoding::reverse(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
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
                response_json["result"] = api_json["result"]["items"][0]["point"];
            }
        );
    }

    callback(HttpResponse::newHttpJsonResponse(response_json));
}
