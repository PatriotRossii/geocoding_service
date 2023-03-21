#pragma once

#include <drogon/utils/coroutine.h>
#include <drogon/nosql/RedisClient.h>

#include <string>

template <typename T>
drogon::Task<std::string> getFromCache(
    const std::string &key,
    const drogon::nosql::RedisClientPtr &client) noexcept(false)
{
    auto value = co_await client->execCommandCoro("get %s", key.data());
    co_return value.asString();
}

template <typename T>
drogon::Task<> updateCache(
    const std::string &key,
    T &&value,
    const drogon::nosql::RedisClientPtr &client) noexcept(false)
{
    co_await client->execCommandCoro("set %s %s", key.data(), value.data());
}
