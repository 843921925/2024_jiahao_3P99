//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "veins/modules/application/traci/TraCIDemo11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

#include "veins/modules/messages/ReportMessage_m.h"
#include "veins/modules/messages/requestM_m.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>


#include "veins/modules/application/traci/ResourcePool.h"
#include "veins/modules/application/traci/Vehicle.h"
#include "veins/modules/application/traci/Request.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);


void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;

    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
//    if (currentSubscribedServiceId == -1) {
////        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
//        currentSubscribedServiceId = wsa->getPsid();
//        if (currentOfferedServiceId != wsa->getPsid()) {
//            stopService();
//            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
//        }
//    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

//    findHost()->getDisplayString().setTagArg("i", 1, "green");

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
//         repeat the received traffic update once in 2 seconds plus some random delay
//        wsm->setSenderAddress(myId);
//        wsm->setSerial(3);
//        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }
}

void TraCIDemo11p::onRM(ReportMessage* rm)
{
    if (rm->getSenderType() == 0) {

        lastReceiveAt = simTime();
    // alter the color of the component by editing the display string
        findHost()->getDisplayString().setTagArg("i", 1, "green");
        ReportMessage* newRM = new ReportMessage();
        requestM* newreqM = new requestM();

        std::uniform_int_distribution<int> resourceDistribution(0, 100);
        resource["application"] = resourceDistribution(generator);
        resource["memory"] = resourceDistribution(generator);
        resource["bandwidth"] = resourceDistribution(generator);
//        std::cout << "***from Car:" << myId << "first_setup_resource"<<std::endl;

        std::uniform_real_distribution<double> timeDistribution(0, 60);
        newRM->setPat(timeDistribution(generator));

 //random requests
        std::uniform_int_distribution<int> reqresourceDistribution(0, 100);
        std::uniform_int_distribution<int> reqDistribution(1, 100);
        int ifProbability = 50; // **% do if
        int random_number = reqDistribution(generator);

            if (random_number <= ifProbability) {
            reqresource["application"] = reqresourceDistribution(generator);
            reqresource["memory"] = reqresourceDistribution(generator);
            reqresource["bandwidth"] = reqresourceDistribution(generator);
            if (reqresource["application"] != 0) {
                resource["application"] = 0;
            }

            if (reqresource["memory"] != 0) {
                resource["memory"] = 0;
            }

            if (reqresource["bandwidth"] != 0) {
                resource["bandwidth"] = 0;
            }

            //set reqresource value to arary ot reqM
                    newreqM->setReqKVPArraySize(reqresource.size());
                            int index2 = 0;
                            for (const auto& kv : reqresource) {
                                reqKVPair reqkvp;
                                reqkvp.setKey(kv.first.c_str());
                                reqkvp.setValue(kv.second);
                                newreqM->setReqKVP(index2++, reqkvp);
                            }


            //        sending reqM
                  populateWSM(newreqM);
                  newreqM->setSenderAddress(myId);
                  newreqM->setSenderType(1);
                  newreqM->setPrt(timeDistribution(generator));
//                  scheduleAt(simTime() + uniform(0.01, 0.2), newreqM);
                  scheduleAt(simTime() + uniform(0.01, 0.2), newreqM->dup());


            }
            else {}

         //set resource value to arary ot Rm
        newRM->setKeyValuePairsArraySize(resource.size());
        int index1 = 0;
        for (const auto& kv : resource) {
            KeyValuePair kvp;
            kvp.setKey(kv.first.c_str());
            kvp.setValue(kv.second);
            newRM->setKeyValuePairs(index1++, kvp);
        }

// //set reqresource value to arary ot reqM
//        newreqM->setReqKVPArraySize(reqresource.size());
//                int index2 = 0;
//                for (const auto& kv : reqresource) {
//                    reqKVPair reqkvp;
//                    reqkvp.setKey(kv.first.c_str());
//                    reqkvp.setValue(kv.second);
//                    newreqM->setReqKVP(index2++, reqkvp);
//                }

//        sending RM
        populateWSM(newRM);
        newRM->setSenderAddress(myId);
        newRM->setSenderType(1);
//        scheduleAt(simTime() + uniform(0.01, 0.2), newRM);
        scheduleAt(simTime() + uniform(0.01, 0.2), newRM->dup());
        delete newRM;
        delete newreqM;

    }

//    else if(rm->getSenderType() == 1){
//        if (rm->getSerial() >= 3) {
//
//        } else {
//            rm->setSerial(rm->getSerial() + 1);
//
//            sendDelayedDown(rm->dup(), 1 + uniform(0.01, 0.2));
//        }
//    }
}

void TraCIDemo11p::onReqM(requestM* reqM) {
    if (reqM->getSenderType() == 1) {
        if (reqM->getSerial() >= 2) {
            if (reqM->isScheduled()) {
                cancelEvent(reqM);
            }
//            delete reqM;
        } else {
            reqM->setSerial(reqM->getSerial() + 1);
            if (reqM->isScheduled()) {
                cancelEvent(reqM);
            }
            sendDelayedDown(reqM->dup(), 1 + uniform(0.01, 0.2));
        }
    }
}


void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
//        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
//            scheduleAt(simTime() + 1, wsm);
        }
    }

    else if (ReportMessage* rm = dynamic_cast<ReportMessage*>(msg)) {
            rm->setSenderPos(curPosition);
//            rm->setSerial(1);
//            rm->setId(simTime().str().c_str());
            sendDown(rm->dup());
//            sendDelayedDown(rm->dup(),simTime()+ uniform(0.01, 0.2));
//            //std::cout << "***RM send from Car:" << myId << "    time:"<<simTime()<<std::endl;
            delete rm;
        }
    else if (requestM* reqM = dynamic_cast<requestM*>(msg)) {
        reqM->setSenderPos(curPosition);
        reqM->setSerial(1);
        reqM->setId(simTime().str().c_str());
        sendDown(reqM->dup());
//        sendDelayedDown(reqM->dup(),simTime()+ uniform(0.01, 0.2));
                //std::cout << "---reqRM send from Car:" << myId << "    time:"<<simTime()<<std::endl;
                delete reqM;
            }

    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    //Editing handlePositionUpdate

//    ReportMessage* rm = new ReportMessage();
//    populateWSM(rm);
//    rm->setSenderPos(curPosition);
//    rm->setSenderAddress(myId);
//    sendDelayedDown(rm, 8 + uniform(0.01, 0.2));

    //std::cout << "RM send from Car:" << myId << std::endl;

    // stopped for for at least 10s?
//    if (mobility->getSpeed() < 1) {
//        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
//            findHost()->getDisplayString().setTagArg("i", 1, "red");
//            sentMessage = true;
//
//            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
//            populateWSM(wsm);
//            wsm->setDemoData(mobility->getRoadId().c_str());
//
//            // host is standing still due to crash
//            if (dataOnSch) {
//                startService(Channel::sch2, 42, "Traffic Information Service");
//                // started service and server advertising, schedule message to self to send later
//                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
//            }
//            else {
//                // send right away on CCH, because channel switching is disabled
//                sendDown(wsm);
//            }
//        }
//    }
//    else {
//        lastDroveAt = simTime();
//    }

    if (simTime() - lastReceiveAt >= 3) {
        // alter the color of the component by editing the display string
            findHost()->getDisplayString().setTagArg("i", 1, "white");
        }

}




