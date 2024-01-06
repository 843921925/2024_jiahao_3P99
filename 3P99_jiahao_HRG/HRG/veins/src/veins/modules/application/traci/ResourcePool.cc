#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include "ResourcePool.h"
#include "Vehicle.h"
#include "veins/modules/application/traci/ResourcePool.h"
#include "veins/base/utils/SimpleAddress.h"

//*****************************************************
/// jiahao pang
/// for complie: g++ -std=c++11 *.cpp
//*****************************************************

ResourcePool::ResourcePool()
{

}

ResourcePool::~ResourcePool()
{

}

//std::unordered_map<std::string, std::set<std::string>> requestToVehicles;

void ResourcePool::genK() {
//        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dis1(x10, y10);
        std::normal_distribution<> dis2(x20, y20);
        std::normal_distribution<> dis3(x30, y30); //(mean,standard deviation)

        k1 = dis1(gen);
        k2 = dis2(gen);
        k3 = dis3(gen);

        k1_history.push_back(k1);
        k2_history.push_back(k2);
        k3_history.push_back(k3);

    }

void ResourcePool::setPa(double x100, double y100, double x200, double y200, double x300, double y300){
    x1 = x10;
    x2 = x20;
    x3 = x30;
    y1 = y10;
    y2 = y20;
    y3 = y30;

    x10 = x100;
    x20 = x200;
    x30 = x300;
    y10 = y100;
    y20 = y200;
    y30 = y300;

}

void ResourcePool::addVehicle(Vehicle *vehicle)
{
    this->pool.push_back({vehicle->vehicleID, *vehicle});
    this->dictionary.insert({vehicle->vehicleID, *vehicle});
}

void ResourcePool::removeVehicle(Vehicle *vehicle)
{
    this->dictionary.erase(vehicle->vehicleID);
    auto it = std::find_if(this->pool.begin(), this->pool.end(),
                      [&vehicle](const std::pair<std::string, Vehicle>& p) {
                      return p.first == vehicle->vehicleID;
                               });
        if (it != this->pool.end()) {
            this->pool.erase(it);
        }
}

void ResourcePool::sortVehicleByRequest(Request *request)
{
    std::unordered_map<std::string, double> vehicleScore;
    std::vector<std::pair<std::string, Vehicle>> temPool = this->pool;
    std::unordered_map<std::string, Vehicle> temMap = this->dictionary;

    //Sort vehicle by resource
    //std::vector<std::pair<std::string, Vehicle>> resourceElements(this->pool.begin(), this->pool.end());
    
    for(auto& item : temPool)
    {
        Vehicle *vehicle = &(item.second);
        vehicle->getResourceScore(*request);
    }
    std::vector<std::pair<std::string, Vehicle>> resourceElements(temPool.begin(), temPool.end());
    std::sort(resourceElements.begin(), resourceElements.end(), ResourcePool::sortByResourceScore);

    double n = 1;
    for(auto&item : resourceElements){
        std::string vehicleID = item.first;
        vehicleScore.insert({vehicleID, n*k1});
        //std::cout <<"vehicle score(resource)#1:" << vehicleScore[vehicleID] << "  ID:" << vehicleID<< std::endl;
        n++;
    }

    //Sort vehicle by distance
    //std::vector<std::pair<std::string, Vehicle>> distanceElements(this->pool.begin(), this->pool.end());
//    for(auto& item : this->pool)
//    {
//        Vehicle *vehicle = &(item.second);
//        //std::cout << "request->vehicleID="<<request->vehicleID<<std::endl;
//        Vehicle requestVehicle = this->dictionary.at(request->vehicleID);
//        vehicle->distance2R(requestVehicle);
//    }

    std::vector<std::pair<std::string, Vehicle>> distanceElements(temPool.begin(), temPool.end());

    for (const auto& item : distanceElements) {
            const std::string& vehicleID = item.first;
            const Vehicle& vehicle = item.second;
            //std::cout << "Vehicle ID: " << vehicleID << " - distance: " << vehicle.dist << std::endl;
        }

    std::sort(distanceElements.begin(), distanceElements.end(), ResourcePool::sortByDistance);

    n = 1;
    for(auto& item : distanceElements)
    {
        std::string vehicleID = item.first;
        vehicleScore[vehicleID] = vehicleScore[vehicleID]+n*k2;
                //std::cout <<"vehicle score(distance)#2:" << vehicleScore[vehicleID] << "  ID:" << vehicleID<< std::endl;

        n++;
    }

    //Sort vehicle by predict stay (prStTime)time
    std::vector<std::pair<std::string, Vehicle>> prStTimeElements(temPool.begin(), temPool.end());

    for (const auto& item : prStTimeElements) {
        const std::string& vehicleID = item.first;
        const Vehicle& vehicle = item.second;
        //std::cout << "Vehicle ID: " << vehicleID << " - Stay Time: " << vehicle.stayTime << std::endl;
    }

        std::sort(prStTimeElements.begin(), prStTimeElements.end(), ResourcePool::sortByprStTime);

        n = 1;
        for(auto& item : prStTimeElements)
        {
            std::string vehicleID = item.first;
            vehicleScore[vehicleID] = vehicleScore[vehicleID]+n*k3;
                    //std::cout <<"vehicle score(prStTime)#3:" << vehicleScore[vehicleID] << "  ID:" << vehicleID<< std::endl;

            n++;
        }



    //sort vehicleScore
    std::vector<std::pair<std::string, double>> vehicleScoreElements(vehicleScore.begin(), vehicleScore.end());
    std::sort(vehicleScoreElements.begin(), vehicleScoreElements.end(), ResourcePool::sortVehicleScore);
    //re-arrange this->pool
    std::vector<std::pair<std::string, Vehicle>> newPool;
    std::vector<std::pair<std::string, Vehicle>> tempPool2;
    for(auto& item : vehicleScoreElements)
    {
        std::string vehicleID = item.first;
 //       std::cout << "Vehicle ID: " << vehicleID << " - " << item.second << std::endl;
//        Vehicle v = this->dictionary.at(vehicleID);
//        newPool.push_back({vehicleID, v});
        auto it8 = temMap.find(vehicleID);
        if (it8 != temMap.end()) {
            // safe it->second
            Vehicle v = it8->second;
            newPool.push_back({vehicleID, v});
        } else {

            std::cerr << "Error: Vehicle ID " << vehicleID << " not found in dictionary." << std::endl;
        }
    }

    //change pool
    for (const auto& newPair : newPool) {
        auto it = std::find_if(this->pool.begin(), this->pool.end(),
                               [&newPair](const std::pair<std::string, Vehicle>& oldPair) {
                                   return oldPair.first == newPair.first;
                               });
        if (it != this->pool.end()) {
            tempPool2.push_back(*it);
        }
    }

    //
    for (const auto& oldPair : this->pool) {
        auto it = std::find_if(tempPool2.begin(), tempPool2.end(),
                               [&oldPair](const std::pair<std::string, Vehicle>& tempPair) {
                                   return tempPair.first == oldPair.first;
                               });
        if (it == tempPool2.end()) {
            tempPool2.push_back(oldPair);
        }
    }
    //print old pool
    for(auto& item : this->pool)
        {
            Vehicle *vehicle = &(item.second);
            //std::cout << "RE-ARRANGE: vehicleID=" << vehicle->vehicleID << std::endl;
        }
    //std::cout << "****************"<< std::endl;
    this->pool = tempPool2;

//    this->pool = newPool;

    //print new pool
    for(auto& item : this->pool)
    {
        Vehicle *vehicle = &(item.second);
       // std::cout << "RE-ARRANGE: vehicleID=" << vehicle->vehicleID << std::endl;
    }
    //std::cout << "----------------------------------------------------------------------"<< std::endl;


}

bool ResourcePool::sortByResourceScore(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2)
{
    return (v1.second.resourceScore < v2.second.resourceScore);
}

bool ResourcePool::sortByDistance(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2)
{
    return (v1.second.dist < v2.second.dist);
}

bool ResourcePool::sortByprStTime(std::pair<std::string, Vehicle> v1, std::pair<std::string, Vehicle> v2)
{
    return (v1.second.stayTime > v2.second.stayTime);
}

bool ResourcePool::sortVehicleScore(std::pair<std::string, int> v1, std::pair<std::string, int> v2)
{
    return (v1.second < v2.second);
}

void ResourcePool::updateRequestAssignment(long reqID,const std::string& requestId, const std::string& vehicleId) {
        requestToVehicles[reqID].insert(vehicleId);
        requestToVehicles[reqID].insert(requestId);
        isRequestToVehicles = true;
    }

//bool ResourcePool::isRequestServed(const std::string& requestId, const std::map<veins::LAddress::L2Type, simtime_t>& connectedNodes) {
//        if (requestToVehicles.find(requestId) == requestToVehicles.end()) {
//            return true; // Requests without assigned vehicles are considered serviced by default
//        }
//
//        for (const auto& vehicleId : requestToVehicles[requestId]) {
//            if (connectedNodes.find(std::stoi(vehicleId)) == connectedNodes.end()) {
//                return false; // If any vehicle leaves the RSU, returns false
//            }
//        }
//
//        return true; // All vehicles are covered by RSU
//    }

//Sorting!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Request ResourcePool::requestResource(Request *request)
{
    std::unordered_map<std::string, double> remainResource;
    // std::cout << "Resource available" << std::endl;

    genK();
    this->sortVehicleByRequest(request);
    for(auto& item : request->resource)
    {
        std::string name = item.first;
        double qty = item.second;
        double remain = qty;
        //for each vehicle in the pool
        for(auto& item : this->pool)
        {
            Vehicle *vehicle = &(item.second);
//            std::cout << "DBG: vehicleID=" << vehicle->vehicleID << std::endl;
            remain = vehicle->assign(name, remain);
            //updateRequestAssignment
            updateRequestAssignment(reqCount,request->vehicleID, vehicle->vehicleID);
//            std::cout << "DBG: resource assigned" << std::endl;
            if(remain == 0) break;
        }
        if(remain != 0){
            remainResource.insert({name, remain});
        }

    }
    Request remainReq(request->vehicleID, &remainResource);
    return remainReq;
}

Request ResourcePool::releaseResource(Request *request)
{
    std::unordered_map<std::string, double> remainResource;
    for(auto& item : request->resource)
    {
        std::string name = item.first;
        double qty = item.second;
        double remain = qty;
        //for each vehicle in the pool
        for(auto& item : this->pool)
        {
            Vehicle *vehicle = &(item.second);
            //std::cout << "DBG: vehicleID=" << vehicle->vehicleID << std::endl;
            remain = vehicle->dismiss(name, remain);
//            std::cout << "resource released" << std::endl;
            if(remain == 0) break;
        }
        if(remain != 0){
            remainResource.insert({name, remain});
        }

    }
    Request remainReq(request->vehicleID,&remainResource);
    return remainReq;
}

double ResourcePool::peekResource(std::string resourceName)
{
    double qty = 0;
    for(auto& item : this->pool)
    {
        Vehicle vehicle = item.second;
        for(auto& resource : vehicle.resource)
        {
            if(resource.first.compare(resourceName) == 0)
            {
                qty += resource.second;
            }
        }
    }
    return qty;
}

double ResourcePool::peekUsedResource(std::string resourceName)
{
    double qty = 0;
    for(auto& item : this->pool)
    {
        Vehicle vehicle = item.second;
        for(auto& resource : vehicle.usedResource)
        {
            if(resource.first.compare(resourceName) == 0)
            {
                qty += resource.second;
            }
        }
    }
    return qty;
}

std::set<std::string> ResourcePool::getAllNames()
{
    std::set<std::string> list;
    for(auto& item : this->pool)
    {
        Vehicle vehicle = item.second;
        for(auto& resource : vehicle.resource)
        {
            list.insert(resource.first);
        }
    }
    return list;
}

Vehicle ResourcePool::getVehicleByID(std::string vehicleID)
{
    return this->dictionary.at(vehicleID);
}

double ResourcePool::calculateMean(const std::vector<double>& values) {
    if (values.empty()) return 0.0;

    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double ResourcePool::calculateStdDev(const std::vector<double>& values) {
    double mean = calculateMean(values);
    double sq_sum = std::accumulate(values.begin(), values.end(), 0.0,
        [mean](double accumulator, double value) {
            return accumulator + (value - mean) * (value - mean);
        });
    return std::sqrt(sq_sum / values.size());
}
