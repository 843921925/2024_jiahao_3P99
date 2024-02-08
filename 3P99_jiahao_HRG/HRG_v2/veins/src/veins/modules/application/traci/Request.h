#ifndef REQUEST_H
#define REQUEST_H

#include <unordered_map>
#include <string>
#include <vector>
//*****************************************************
/// jiahao pang
/// for complie: g++ -std=c++11 *.cpp
//*****************************************************

struct Request
{
    std::string vehicleID;
    std::unordered_map<std::string, double> resource;
    Request(std::string vehicleID, std::unordered_map<std::string, double> *resource)
    {
        this->vehicleID = vehicleID;
        this->resource = *resource;
    }
};
#endif
