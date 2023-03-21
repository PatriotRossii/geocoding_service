#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class Geocoding : public drogon::HttpController<Geocoding>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Geocoding::forward, "/forward", Get);
    METHOD_ADD(Geocoding::reverse, "/reverse", Get);
    METHOD_LIST_END

    drogon::AsyncTask forward(HttpRequestPtr req, std::function<void (const HttpResponsePtr &)> callback);
    void reverse(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
};
