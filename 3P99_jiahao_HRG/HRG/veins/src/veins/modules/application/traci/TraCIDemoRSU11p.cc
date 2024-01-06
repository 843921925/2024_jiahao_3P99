//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/TraCIDemoRSU11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

#include "veins/modules/messages/ReportMessage_m.h"
#include "veins/modules/messages/requestM_m.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include "veins/modules/application/traci/ResourcePool.h"
#include "veins/modules/application/traci/Vehicle.h"
#include "veins/modules/application/traci/Request.h"

using namespace veins;

Define_Module(veins::TraCIDemoRSU11p);
//ResourcePool pool;
//double calculateTimeToEdge(const Coord& r, double x1, double y1, double x2, double y2, double time, double radius);
//int falseCount = 0;
//int slotReqCount = 0;
//double oldFalseR = 0;
//cMessage* cleanupTimer; // timer message
//double cleanupInterval = 3; // clean interval



void TraCIDemoRSU11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
//    if (stage == 0) {
//        scheduleAt(simTime() + uniform(0.01, 0.2), new cMessage("Connected nodes"));
//    }

    if (stage == 1) {

        cModule* currentModule = this;
        cModule* connectionManagerModule = nullptr;
           while (currentModule != nullptr) {
               connectionManagerModule = currentModule->getModuleByPath("^.connectionManager");
               if (connectionManagerModule != nullptr) {
                   break;
               }
               currentModule = currentModule->getParentModule();
           }

           if (connectionManagerModule != nullptr && connectionManagerModule->hasPar("maxInterfDist")) {
               maxInterferenceDistance = connectionManagerModule->par("maxInterfDist").doubleValue();
               EV << "Max Interference Distance: " << maxInterferenceDistance << "m" << endl;
           } else {
               EV << "Connection Manager module not found or parameter 'maxInterfDist' missing." << endl;
           }

            ReportMessage* rm = new ReportMessage();
            populateWSM(rm);
            rm->setSenderAddress(myId);
            rm->setSenderType(0);
            scheduleAt(simTime() + 2 + uniform(0.01, 0.2), rm);

            requestM* actCacul = new requestM("caculateFalseRate");
            populateWSM(actCacul);


            pool.setPa(10.2856, 1.29572,  10.1048,1.11644,  9.95373,0.956704);





            scheduleAt(simTime() + 2000 + uniform(0.01, 0.2), actCacul);

            //clean message recorder timmer
            cleanupTimer = new cMessage("cleanupTimer");
            scheduleAt(simTime() + cleanupInterval + uniform(0.01, 0.2), cleanupTimer);

            std::unordered_map<std::string, double> resourcesRSU;
            std::unordered_map<std::string, double> usedResourcesRSU;
            resourcesRSU.insert({"application", 0});
            resourcesRSU.insert({"memory", 0});
            resourcesRSU.insert({"bandwidth", 0});

            usedResourcesRSU.insert({"application", 0});
            usedResourcesRSU.insert({"memory", 0});
            usedResourcesRSU.insert({"bandwidth", 0});

            std::string vehicleId = std::to_string(myId);
            Vehicle* RSU = new Vehicle(vehicleId, &resourcesRSU, &usedResourcesRSU);
            RSU->x = curPosition.x;
            RSU->y = curPosition.y;

//           pool.addVehicle(RSU);

        }

}

void TraCIDemoRSU11p::onWSA(DemoServiceAdvertisment* wsa)
{
    // if this RSU receives a WSA for service 42, it will tune to the chan
//    if (wsa->getPsid() == 42) {
//        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
//    }
}

void TraCIDemoRSU11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    // this rsu repeats the received traffic update in 2 seconds plus some random delay
   // sendDelayedDown(wsm->dup(), 2 + uniform(0.01, 0.2));
}

void TraCIDemoRSU11p::onRM(ReportMessage* frame)
{
    ReportMessage* rm = check_and_cast<ReportMessage*>(frame);
    if(rm->getSenderType() == 1){
//    std::string messageId = rm->getId();
//    if (processedMessagesRM.find(messageId) == processedMessagesRM.end()) {
//            // first receive
//        processedMessagesRM.insert(messageId);
//            // process onRM

    LAddress::L2Type sender = rm->getSenderAddress();
    Coord position =rm->getSenderPos();
   // std::cout <<"**********get RM!!!!!:"<<sender<<"    time:"<<simTime()<<std::endl;
    double x1,y1;
    std::string vehicleId = std::to_string(rm->getSenderAddress());

    //std::cout << "||||||||CarId: " << vehicleId << " x: " << position.x <<"  y:"<<position.y<< std::endl;

    //update resource map by Rm from car
      std::unordered_map<std::string, double> resources;
      std::unordered_map<std::string, double> usedResources;
       unsigned int numPairs = rm->getKeyValuePairsArraySize();
       for (unsigned int i = 0; i < numPairs; ++i) {
                   KeyValuePair kvp = rm->getKeyValuePairs(i);
                   resources.insert({kvp.getKey(),kvp.getValue()});
               }

       //printout reource_map
//                      for (const auto& pair : resources) {
//                              std::cout <<"ID:"<<vehicleId<< "***resource_map_Key: " << pair.first << "  ,Value: " << pair.second << std::endl;
//                          }

       std::unordered_map<std::string, Vehicle>::iterator it2;
       it2 = pool.dictionary.find(vehicleId);
       if(it2 == pool.dictionary.end()) {
               // if not exist, insert
                  Vehicle* vehicles = new Vehicle(vehicleId,&resources,&usedResources);
//                  pool.addVehicle(vehicles);
                  vehicles->x = position.x;
                  vehicles->y = position.y;
                  vehicles->avTime = rm->getPat();
                  vehicles->dist = vehicles->distance2R(curPosition.x,curPosition.y);
                  pool.addVehicle(vehicles);
//
//                  std::cout << "First--CarId: " << vehicleId << std::endl;
//                  for (const auto& pair : resources) {
//                      std::cout << "Resource: " << pair.first << ", Value: " << pair.second << std::endl;
//                  }

       }
              // exist, update
               else {
                   auto it4 = pool.dictionary.find(vehicleId);
                   if (it4 != pool.dictionary.end()) {
//                std::cout << "old*******CarId: " << vehicleId << " x1: " << it4->second.x <<"  y1:"<<it4->second.y<< std::endl;
                        x1 = it4->second.x;
                        y1 = it4->second.y;
                       } else {
                          std::cout << "CarId: " << vehicleId << " not found in dictionary." << std::endl;
                        }
                   double x2 = position.x;
                   double y2 = position.y;
                   double temp = calculateTimeToEdge(curPosition,x1,y1,x2,y2,2,maxInterferenceDistance);
//                   std::cout << "new-----CarId: " << vehicleId << " x2: " << x2 <<"  y2:"<<y2<< std::endl;

                   //update map dictionary Vehical
                   Vehicle& existingVehicle = it2->second;
                       existingVehicle.x = position.x;
                       existingVehicle.y = position.y;
                       existingVehicle.avTime = rm->getPat();
                       existingVehicle.dist = existingVehicle.distance2R(curPosition.x,curPosition.y);
                       existingVehicle.stayTime = temp;
                       for (const auto& newResourcePair : resources) {
                           existingVehicle.resource[newResourcePair.first] = newResourcePair.second;
                       }
//                       for (const auto& newUsedResourcePair : usedResources) {
//                           existingVehicle.usedResource[newUsedResourcePair.first] = newUsedResourcePair.second;
//                       }

                       // update vector pool Vehicle
                       for (auto& pair : pool.pool) {
                           if (pair.first == vehicleId) {
                               pair.second = existingVehicle;
                               break;
                           }
                       }
                       //std::cout << "Vehicle ID:" << vehicleId << " updated" << std::endl;


               }

       auto it3 = pool.dictionary.find(vehicleId);
       if (it3 != pool.dictionary.end()) {
          // std::cout << "-----CarId: " << vehicleId << " x: " << it3->second.x <<"  y:"<<it3->second.y<< std::endl;
       } else {
           std::cout << "CarId: " << vehicleId << " when seting (x,y)not found in dictionary." << std::endl;
       }

       //update X,Y by Rm from car




       //update car list in RSU range
//        LAddress::L2Type sender = rm->getSenderAddress();
        simtime_t time = simTime();
        std::map<LAddress::L2Type, simtime_t>::iterator it;
        it = connectedNodes.find(sender);

        if(it == connectedNodes.end()) {
        // if not exist, insert
            connectedNodes.insert(std::make_pair(sender, time));
        }
        else {
        // exist, update time
            it->second = time;
        }
        //check dictionary add?
//        std::cout << "Vehicle ID:" <<sender <<" add in connectedNodes" << std::endl;
//        auto it4 = pool.dictionary.find(vehicleId);
//                    if (it4 != pool.dictionary.end()) {
//                       // find it
//                        std::cout << "Vehicle ID:" <<vehicleId <<" add in pool" << std::endl;
//
//                    } else {
//                                        // vehicleId not in dictionary
//                         std::cout << "Vehicle ID:" <<vehicleId <<" not found in dictionary, when want checkiinggggg." << std::endl;
//                             }

//        //all  car in pool
//        const auto& v = pool.pool;
//        for (const auto& pair : v) {
//            std::cout << "pool****************Vehicle ID: " << pair.first << std::endl;
//        }
        //all car in connectedNodes
//        for (const auto& pair : connectedNodes) {
//                std::cout <<"RSU_ID:"<<myId<< "--bconnectedNodes--Vehicle ID: " << pair.first <<"-----------------"<< std::endl;
//            }
//        std::cout<<"**************************"<<std::endl;
        // display a textbox above the RSU icon in the map showing the number of connected nodes
        findHost()->getDisplayString().setTagArg("t", 0, connectedNodes.size());

//} else {
//            // message got before
//        }
//delete rm;
    }
}


void TraCIDemoRSU11p::onReqM(requestM* frame){

      requestM* reqM = check_and_cast<requestM*>(frame);

      std::string messageId = reqM->getId();

         if (processedMessagesReqM.find(messageId) == processedMessagesReqM.end()) {

             processedMessagesReqM.insert(messageId);

        LAddress::L2Type sender = reqM->getSenderAddress();
        Coord position =reqM->getSenderPos();
       // std::cout <<"----------get requestMessage!!!!!:"<<sender<<"    time:"<<simTime()<<std::endl;
        std::string vehicleId = std::to_string(reqM->getSenderAddress());

        //transfer reqM array to map reqresource
        std::unordered_map<std::string, double> reqresources;
               unsigned int numPairs = reqM->getReqKVPArraySize();
               for (unsigned int i = 0; i < numPairs; ++i) {
                   reqKVPair reqkvp = reqM->getReqKVP(i);
                           reqresources.insert({reqkvp.getKey(),reqkvp.getValue()});
                       }

               Request request(vehicleId, &reqresources);
               double time = simTime().dbl() + reqM->getPrt();
//request in pool
              Request remainReq = pool.requestResource(&request);

              if(pool.isRequestToVehicles){
              pool.requestTimer.insert({pool.reqCount,time});
              pool.reqRealseMap.insert({pool.reqCount,remainReq});
              pool.isRequestToVehicles = false;
              pool.reqCount++;
              slotReqCount++;
              }
//              pool.reqCount++;
//              slotReqCount++;
//              std::cout <<"RSU_ID:"<<myId<<"------------------------------------------"<<std::endl;
//               for (const auto& pair : pool.requestToVehicles) {
//                       std::cout <<"before check---request pool: "<< pair.first << ": ";
//                       for (const auto& vehicle : pair.second) {
//                           std::cout << vehicle << "-- ";
//                       }
//                       std::cout << std::endl;
//                   }
//
//               for (const auto& pair : pool.requestTimer) {
//                  std::cout <<"before check---"<<"ID:"<<pair.first<< "---time---: " << pair.second <<"--simtime:" <<simTime()<<std::endl;
//                       }

//               std::set<std::string> names = pool.getAllNames();
//                   std::cout << "Resource: " << std::endl;
//                   for(std::string name : names)
//                   {
//                       std::cout << name <<": " << pool.peekResource(name) << std::endl;
//                   }
//
//                   std::cout << "Used Resource: " << std::endl;
//                   for(std::string name : names)
//                   {
//                       std::cout << name <<": " << pool.peekUsedResource(name) << std::endl;
//                   }
//
//                           const auto& v = pool.pool;
//                           for (const auto& pair : v) {
//                               std::cout << "pool****************Vehicle ID: " << pair.first << std::endl;
//                           }
//printout reqreource_map
//               for (const auto& pair : reqresources) {
//                       std::cout <<"RSU_ID"<< myId <<"---car_ID:"<<vehicleId<< "---reqresource_map_Key: " << pair.first << "  ,Value: " << pair.second << std::endl;
//                   }
//std::cout << "*********************************" << std::endl;
         } else {

                  }
//         delete reqM;

}



void TraCIDemoRSU11p::handleSelfMsg(cMessage* msg)
{
//    std::cout << "There are " << connectedNodes.size() << " connected nodes in the range of RSU " << myId << std::endl;
//    scheduleAt(simTime() + 1.5, new cMessage("Connected nodes"));
//    requestM* reqM = dynamic_cast<requestM*>(msg);
//    ReportMessage* rm = dynamic_cast<ReportMessage*>(msg);
    if(ReportMessage* rm = dynamic_cast<ReportMessage*>(msg)){
        scheduleAt(simTime()+ uniform(0.01, 0.2) + 1, rm);
        sendDown(rm->dup());
//        sendDelayedDown(rm->dup(),simTime()+ uniform(0.01, 0.2));
        std::map<LAddress::L2Type, simtime_t>::iterator it;

//        std::cout <<"k1:"<<pool.k1<<"---k2:"<<pool.k2<<"---k3:"<<pool.k3<<std::endl;

        // checking disconnected nodes by iterating the map
        for(it = connectedNodes.begin(); it != connectedNodes.end(); it++) {
        // if the last report time of a specific node is 3 seconds ago, assume it's disconnected
            if (simTime() - it->second >= 3) {

                std::string vehicleId = std::to_string(it->first);

                requestM* carLeaveMsg = new requestM("immediate");
                carLeaveMsg->setDeliverID(vehicleId.c_str());
                scheduleAt(simTime()+uniform(0.01, 0.2), carLeaveMsg);

                connectedNodes.erase(it++);

//               std::cout << "Vehicle ID:" <<vehicleId <<" delete from connectedNodes" <<std::endl;

              //remove car resource when it left
                auto it2 = pool.dictionary.find(vehicleId);
            if (it2 != pool.dictionary.end()) {
               // find it
                Vehicle& vehicles = it2->second;
                pool.removeVehicle(&vehicles);
//                std::cout << "Vehicle ID:" <<vehicleId <<" delete from poolllllll" << std::endl;

//                //all  car in pool
//                        const auto& v = pool.pool;
//                        for (const auto& pair : v) {
//                            std::cout << "pool****************Vehicle ID: " << pair.first << std::endl;
//                        }
//                        //all car in connectedNodes
//                        for (const auto& pair : connectedNodes) {
//                                std::cout << "connectedNodes--Vehicle ID: " << pair.first <<"-----------------"<< std::endl;
//                            }
 //     std::cout << "Vehicle:"<<vehicleId<<" left RUS and removed from resource pool" << std::endl;

//      std::cout <<"update pool after car left |||||||||||||||||||||||||"<< std::endl;
//              std::set<std::string> names = pool.getAllNames();
//                         std::cout << "Resource: " << std::endl;
//                         for(std::string name : names)
//                         {
//                             std::cout << name <<": " << pool.peekResource(name) << std::endl;
//                         }
//
//                         std::cout << "Used Resource: " << std::endl;
//                         for(std::string name : names)
//                         {
//                             std::cout << name <<": " << pool.peekUsedResource(name) << std::endl;
//                         }


            } else {
                                // vehicleId not in dictionary
//                 std::cout << "Vehicle ID:" <<vehicleId <<" not found in dictionary, when car left." << std::endl;
                     }

                if (it == connectedNodes.end()) {
                    break;
                }
            }
        }

        findHost()->getDisplayString().setTagArg("t", 0, connectedNodes.size());

        //self check request pool.
        simtime_t currentTime = simTime();

        for (auto it = pool.requestTimer.begin(); it != pool.requestTimer.end(); ) {
            if (it->second <= currentTime.dbl()) {
                long requestId = it->first;
                pool.requestToVehicles.erase(requestId);
                it = pool.requestTimer.erase(it);

                auto it6 = pool.reqRealseMap.find(requestId);
                if (it6 != pool.reqRealseMap.end()) {
                    Request release = it6->second;
                                Request remainRelease = pool.releaseResource(&release);

//                                    if(!remainRelease.resource.empty()){
//                                        std::cout << "There is some remaining for another pool." << std::endl;
//                                        std::cout << "Remaining unreleased request: " << std::endl;
//                                        for(auto& item : remainRelease.resource)
//                                        {
//                                            std::cout << item.first << ": " << item.second << std::endl;
//                                        }
//                                        std::cout << "==============================" << std::endl;
//                                        // Request remainRelease2 = pool2.releaseResource(&remainRelease);
//
//                                    }else{
//                                        std::cout << "All requested resources are released successfully." << std::endl;
//                                    }

                } else {
//                    std::cout << "not found reqID when relesing request" << std::endl;
                }


            } else {
                ++it;
            }
        }
//        for (const auto& pair : pool.requestToVehicles) {
//                std::cout <<"**after selt check---request pool: "<< pair.first << ": ";
//                for (const auto& vehicle : pair.second) {
//                    std::cout << vehicle << "-- ";
//                }
//                std::cout << std::endl;
//            }
//
//        for (const auto& pair : pool.requestTimer) {
//           std::cout <<"**after selt check---"<<"ID:"<<pair.first<< "---time---: " << pair.second <<"--simtime:" <<simTime()<<std::endl;
//                }
    }
//    std::cout <<"*******-----------------------------********"<<std::endl;

//delete request by leave car
if(requestM* reqM = dynamic_cast<requestM*>(msg)){
        if (reqM != nullptr && std::string(reqM->getName()) == "immediate") {

//            for (const auto& pair : pool.requestToVehicles) {
//                                    std::cout <<"**before leave check---request pool: "<< pair.first << ": ";
//                                    for (const auto& vehicle : pair.second) {
//                                        std::cout << vehicle << "-- ";
//                                    }
//                                    std::cout << std::endl;
//                                }
//
//                            for (const auto& pair : pool.requestTimer) {
//                               std::cout <<"**before leave check---"<<"ID:"<<pair.first<< "---time---: " << pair.second <<"--simtime:" <<simTime()<<std::endl;
//                                    }
//                            std::cout <<"***************"<<std::endl;

            std::string leaveId = reqM->getDeliverID();
            long tempId;
            for (auto it = pool.requestToVehicles.begin(); it != pool.requestToVehicles.end(); ) {
                if (it->second.find(leaveId) != it->second.end()) {

                    long requestID = it->first;
                  // std::cout <<"get message!!!!!!!!!!!!!!!**************"<< std::endl;
                    it = pool.requestToVehicles.erase(it);
                    pool.requestTimer.erase(requestID);
                    tempId=requestID;
                    falseCount++;
//                    std:: cout<<"falseCount:"<<falseCount<<std::endl;
                }

                else {
                    ++it;
                }
//                falseCount++;

            }

//            std::cout <<"get message!!!!!!!!!!!!!!!------------------"<< std::endl;
//            for (const auto& pair : pool.requestToVehicles) {
//                                    std::cout <<"**after leave check---request pool: "<< pair.first << ": ";
//                                    for (const auto& vehicle : pair.second) {
//                                        std::cout << vehicle << "-- ";
//                                    }
//                                    std::cout << std::endl;
//                                }

//                            for (const auto& pair : pool.requestTimer) {
//                               std::cout <<"**after leave check---"<<"ID:"<<pair.first<< "---time---: " << pair.second <<"--simtime:" <<simTime()<<std::endl;
//                                    }
//                            std::cout <<"-------------------------------------------"<<std::endl;

            auto it7 = pool.reqRealseMap.find(tempId);
                            if (it7 != pool.reqRealseMap.end()) {
                                Request release = it7->second;
                                            Request remainRelease = pool.releaseResource(&release);

//                                                if(!remainRelease.resource.empty()){
//                                                    std::cout << "There is some remaining for another pool." << std::endl;
//                                                    std::cout << "Remaining unreleased request: " << std::endl;
//                                                    for(auto& item : remainRelease.resource)
//                                                    {
//                                                        std::cout << item.first << ": " << item.second << std::endl;
//                                                    }
//                                                    std::cout << "==============================" << std::endl;
//                                                    // Request remainRelease2 = pool2.releaseResource(&remainRelease);
//
//                                                }else{
//                                   std::cout << "All requested resources are released successfully." << std::endl;
//                                                }

                            } else {
//                                std::cout << "not found reqID when relesing request" << std::endl;
                            }

        }

}

if(requestM* reqM = dynamic_cast<requestM*>(msg)){
    if (reqM != nullptr && std::string(reqM->getName()) == "caculateFalseRate"){
        requestM* cacul = new requestM("caculateFalseRate");
        scheduleAt(simTime() +2000 +uniform(0.01, 0.2), cacul);
        double falseRate = falseCount/(slotReqCount * 1.0);
//        std::cout<<falseRate<< std::endl;
        std::cout<<"RSU_ID:"<<myId<<"  falseRate:"<<falseRate<< std::endl;
        std::cout <<"falseCount:"<<falseCount<<std::endl;
        std::cout <<"slotReqCount:"<<slotReqCount<<std::endl;
        std::cout <<"--------------------------------------"<<std::endl;
        if(oldFalseR != 0){
            if(oldFalseR > falseRate){
                double Gra =  oldFalseR - falseRate;
                double a = 100;   //update rate
                double x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
                double x100 = 0.0, y100 = 0.0, x200 = 0.0, y200 = 0.0, x300 = 0.0, y300 = 0.0;

                x1 = pool.calculateMean(pool.k1_history);
                y1 = pool.calculateStdDev(pool.k1_history);
                x100 = pool.x10+a*Gra*(x1-pool.x10);
                y100 = pool.y10+a*Gra*(y1-pool.y10);

                x2 = pool.calculateMean(pool.k2_history);
                y2 = pool.calculateStdDev(pool.k2_history);
                x200 = pool.x20+a*Gra*(x2-pool.x20);
                y200 = pool.y20+a*Gra*(y2-pool.y20);

                x3 = pool.calculateMean(pool.k3_history);
                y3 = pool.calculateStdDev(pool.k3_history);
                x300 = pool.x30+a*Gra*(x3-pool.x30);
                y300 = pool.y30+a*Gra*(y3-pool.y30);

                pool.setPa(x100,y100,x200,y200,x300,y300);
//print k history
//                std::cout << "k1_history: ";
//                    for (const double& value : pool.k1_history) {
//                        std::cout << value << " ";
//                    }
//                    std::cout << std::endl;
//
//                    std::cout << "k2_history: ";
//                        for (const double& value : pool.k2_history) {
//                            std::cout << value << " ";
//                        }
//                        std::cout << std::endl;
//
//                        std::cout << "k3_history: ";
//                            for (const double& value : pool.k3_history) {
//                                std::cout << value << " ";
//                            }
//                            std::cout << std::endl;

//print k mean
//                            std::cout << "k1_mean: "<< x1 <<std::endl;
//                            std::cout << "k2_mean: "<< x2 <<std::endl;
//                            std::cout << "k3_mean: "<< x3 <<std::endl;
//                std::cout <<"new_pa: ("<< x100 <<", "<<y100<<",    "<<x200 <<", "<<y200<<",    "<<x300 <<", "<<y300<<")"<<std::endl;
                            std::cout << "new_Pa(x1,y1): "<< x100 <<"," << y100 <<std::endl;
                            std::cout << "new_Pa(x2,y2): "<< x200 <<"," << y200 <<std::endl;
                            std::cout << "new_Pa(x3,y3): "<< x300 <<"," << y300 <<std::endl;
                            std::cout <<"-----------------------"<<std::endl;

                pool.k1_history.clear();
                pool.k2_history.clear();
                pool.k3_history.clear();

            }

            else if(oldFalseR < falseRate){
                double Gra =  falseRate - oldFalseR;
                                double a = 100;   //update rate
                                double x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
                                double x100 = 0.0, y100 = 0.0, x200 = 0.0, y200 = 0.0, x300 = 0.0, y300 = 0.0;

                                x1 = pool.calculateMean(pool.k1_history);
                                y1 = pool.calculateStdDev(pool.k1_history);
                                x100 = pool.x10-a*Gra*(x1-pool.x10);
                                y100 = pool.y10-a*Gra*(y1-pool.y10);

                                x2 = pool.calculateMean(pool.k2_history);
                                y2 = pool.calculateStdDev(pool.k2_history);
                                x200 = pool.x20-a*Gra*(x2-pool.x20);
                                y200 = pool.y20-a*Gra*(y2-pool.y20);

                                x3 = pool.calculateMean(pool.k3_history);
                                y3 = pool.calculateStdDev(pool.k3_history);
                                x300 = pool.x30-a*Gra*(x3-pool.x30);
                                y300 = pool.y30-a*Gra*(y3-pool.y30);

                                pool.setPa(x100,y100,x200,y200,x300,y300);

//                               std::cout << "k1_mean: "<< x1 <<std::endl;
//                               std::cout << "k2_mean: "<< x2 <<std::endl;
//                               std::cout << "k3_mean: "<< x3 <<std::endl;
//
//                               std::cout <<"new_pa: ("<< x100 <<", "<<y100<<",    "<<x200 <<", "<<y200<<",    "<<x300 <<", "<<y300<<")"<<std::endl;
                               std::cout << "new_Pa(x1,y1): "<< x100 <<"," << y100 <<std::endl;
                               std::cout << "new_Pa(x2,y2): "<< x200 <<"," << y200 <<std::endl;
                               std::cout << "new_Pa(x3,y3): "<< x300 <<"," << y300 <<std::endl;
                               std::cout <<"-----------------------"<<std::endl;

                               pool.k1_history.clear();
                               pool.k2_history.clear();
                               pool.k3_history.clear();
            }

        }
         oldFalseR = falseRate;

        falseCount = 0;
        slotReqCount = 0;
    }
}

if (msg == cleanupTimer) {
//    processedMessagesRM.clear();
    processedMessagesReqM.clear();
        scheduleAt(simTime() + cleanupInterval + uniform(0.01, 0.2), cleanupTimer);
    }



}

double TraCIDemoRSU11p::calculateTimeToEdge(const Coord& r, double x1, double y1, double x2, double y2, double time, double radius) {
    double m = (y2 - y1) / (x2 - x1); // slope
    double c = y1 - m * x1;

    double a = 1 + m * m;
    double b = 2 * m * c - 2 * r.x - 2 * m * r.y;
    double c_ = r.x * r.x + c * c - 2 * c * r.y + r.y * r.y - radius * radius;

    double discriminant = b * b - 4 * a * c_;
    if (discriminant < 0) {
        std::cout << "No intersection with the circle." << std::endl;
        return 0; // not  touch points
    }

    double intersecX1 = (-b + std::sqrt(discriminant)) / (2 * a);
    double intersecX2 = (-b - std::sqrt(discriminant)) / (2 * a);

    double intersecY1 = m * intersecX1 + c;
    double intersecY2 = m * intersecX2 + c;

    double dist1 = std::sqrt((intersecX1 - x1) * (intersecX1 - x1) + (intersecY1 - y1) * (intersecY1 - y1));
    double dist2 = std::sqrt((intersecX1 - x2) * (intersecX1 - x2) + (intersecY1 - y2) * (intersecY1 - y2));
    double distToEdge=0;
    if(dist1 > dist2){
        distToEdge = dist2;
    }
    else{
        distToEdge = std::sqrt((intersecX2 - x2) * (intersecX2 - x2) + (intersecY2 - y2) * (intersecY2 - y2));
    }

    double dist = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    double v = dist / time;

    return distToEdge / v; // how much time to left
}
