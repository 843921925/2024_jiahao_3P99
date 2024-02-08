#ifndef VEHICLE_H
#define VEHICLE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <cmath>

#include "ResourcePool.h"
#include "Vehicle.h"
#include "Request.h"

//*****************************************************
/// jiahao pang
/// for complie: g++ -std=c++11 *.cpp
//*****************************************************

struct Vehicle
{
    std::string vehicleID;
    double x, y;
    double resourceScore;
    double dist;
    double stayTime;
    double avTime;


    std::unordered_map<std::string, double> resource;
    std::unordered_map<std::string, double> usedResource;
    Vehicle(std::string vehicleID, std::unordered_map<std::string, double> *resource, std::unordered_map<std::string, double> *usedResource)
    {
        this->vehicleID = vehicleID;
        this->resource = *resource;
        this->usedResource = *usedResource;
    }

    //return the remaining qty after assign to this vehicle
    double assign(std::string name, double qty)
    {
        double total = 0;
        auto totalItem = resource.find(name);
        if(totalItem != resource.end()){
            total = totalItem->second;
        }

        double used = 0;
        auto usedItem = usedResource.find(name);
        if(usedItem != usedResource.end()){
            used = usedItem->second;
        }else{
            usedResource.insert({name, 0});
        }

//        double available = total - used;
        double available = total;
        // std::cout << "DBG: available=" << available << std::endl; 
        // std::cout << "DBG: qty=" << qty << std::endl; 
        usedItem = usedResource.find(name);
        if(available >= qty){
            // std::cout << "DBG: usedItem->second=" << usedItem->second << std::endl; 
            usedItem->second += qty;
            return 0;
        }else{
            usedItem->second += available;
            return qty-available;
        }

    }

    double dismiss(std::string name, double qty)
    {
        auto usedItem = usedResource.find(name);
        if(usedItem != usedResource.end())
        {
            if(usedItem->second >= qty){
                usedItem->second -= qty;
            }else{
                double remain = qty - usedItem->second;
                usedItem->second = 0;
                return remain;
            }
            
        }else{
//            std::cerr << "No used resource to dismiss on vehicle " << this->vehicleID << std::endl;
            return qty;
        }
        return 0;
    }

    double distance(Vehicle v2)
    {
        double dist=0;
        if(v2.vehicleID==this->vehicleID)
        {
            dist += 100000; 
        }
        dist = dist + std::sqrt(std::pow(this->x - v2.x, 2) + std::pow(this->y - v2.y, 2));
        this->dist = dist;
//        std::cout <<"distance :" << dist << std::endl;
        return dist;
    }

    double distance2R(double x2, double y2)
    {
        double dist=0;

        dist = dist + std::sqrt(std::pow(this->x - x2, 2) + std::pow(this->y - y2, 2));
        this->dist = dist;
        //std::cout <<"distance2R :" << dist << "  ID:" << this->vehicleID <<std::endl;
        return dist;
    }

    double getResourceScore(Request request)
    {
        double score = 0;
        for(auto& item : request.resource)
        {
            std::string name = item.first;
            double qty = item.second;
//            double vehicleQty = this->resource.at(name) - this->usedResource.at(name);
            double vehicleQty = this->resource.at(name);
            double total = vehicleQty - qty;
            if(total < 0){
                total = std::abs(total) + 100000;
            }
            score += total;
        }
        this->resourceScore = score;
        //std::cout <<"resourse score:" << score << "  ID:" << this->vehicleID << std::endl;
        return score;

        
    }




};
#endif
