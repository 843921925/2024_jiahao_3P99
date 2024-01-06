
#ifndef RESOURCE_POOL_H
#define RESOURCE_POOL_H

#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include <random>
#include "ResourcePool.h"
#include "Vehicle.h"
#include "Request.h"

#include "veins/base/utils/SimpleAddress.h"

//*****************************************************
/// jiahao pang
/// for complie: g++ -std=c++11 *.cpp
//*****************************************************

class ResourcePool
{
    public:

        ResourcePool();
        
        ~ResourcePool();

        void addVehicle(Vehicle *vehicle);

        void removeVehicle(Vehicle *vehicle);

        void sortVehicleByRequest(Request *request);

        void updateRequestAssignment(long reqID,const std::string& requestId, const std::string& vehicleId);

        void genK();

        void setPa(double x100, double y100, double x200, double y200, double x300, double y300);

        Request requestResource(Request *request);

        Request releaseResource(Request *request);

        double peekResource(std::string resourceName);

        double peekUsedResource(std::string resourceName);

        double calculateMean(const std::vector<double>& values);

        double calculateStdDev(const std::vector<double>& values);

//        bool isRequestServed(const std::string& requestId, const std::map<veins::LAddress::L2Type, simtime_t>& connectedNodes);

        std::set<std::string> getAllNames();

        Vehicle getVehicleByID(std::string vehicleID);

        static bool sortByResourceScore(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2);

        static bool sortByDistance(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2);

        static bool sortByprStTime(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2);
    
        static bool sortVehicleScore(std::pair<std::string, int> v1, std::pair<std::string, int> v2);
    public:
        
        std::vector<std::pair<std::string, Vehicle>> pool;
        std::unordered_map<std::string, Vehicle> dictionary;
        std::unordered_map<long, std::set<std::string>> requestToVehicles;
        std::unordered_map<long, double> requestTimer;
        std::unordered_map<long, Request> reqRealseMap;
        std::random_device rd;

        std::vector<double> k1_history;
        std::vector<double> k2_history;
        std::vector<double> k3_history;

        long reqCount = 1;
        bool isRequestToVehicles = false;
        double k1=10;
        double k2=10;
        double k3=10;
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
        double x10 = 0.0, y10 = 0.0, x20 = 0.0, y20 = 0.0, x30 = 0.0, y30 = 0.0;


};
#endif
