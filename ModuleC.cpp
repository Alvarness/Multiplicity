/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file ModuleC.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "ModuleC.h"

using namespace std;

ModuleC::ModuleC(const char * name, const char * description):JPetTask(name, description){}

ModuleC::~ModuleC(){}

void ModuleC::init(const JPetTaskInterface::Options& opts){
}

void ModuleC::exec(){
    //getting the data from event in propriate format
    if(auto currSignal = dynamic_cast<const JPetRawSignal*const>(getEvent())){

        getStatistics().getCounter("No. initial signals")++;

        if (fSignals.empty()) {
            fSignals.push_back(*currSignal);
        }

        else {
            if (fSignals[0].getTimeWindowIndex() == currSignal->getTimeWindowIndex()) {
                fSignals.push_back(*currSignal);
            }
            else {
                saveHits(createHits(fSignals));
                fSignals.clear();
                fSignals.push_back(*currSignal);
            }
        }
    }
}

vector<JPetHit> ModuleC::createHits(const vector<JPetRawSignal>&signals){
    vector<JPetHit> hits;
    for (auto i = signals.begin(); i != signals.end(); ++i) {
        for (auto j = i; ++j != signals.end();) {
            if (i->getPM().getScin() == j->getPM().getScin()) {
                // found 2 signals from the same scintillator
                // wrap the RawSignal objects into RecoSignal and PhysSignal
                // for now this is just wrapping opne object into another
                // in the future analyses it will involve more logic like
                // reconstructing the signal's shape, charge, amplitude etc.
                JPetRecoSignal recoSignalA;
                JPetRecoSignal recoSignalB;
                JPetPhysSignal physSignalA;
                JPetPhysSignal physSignalB;
                // assign sides A and B properly
                if( i -> getNumberOfTrailingEdgePoints() == 4 and j -> getNumberOfTrailingEdgePoints() == 4
                   and i -> getNumberOfLeadingEdgePoints() == 4 and j -> getNumberOfLeadingEdgePoints() == 4){

                    if(
                        (i->getPM().getSide() == JPetPM::SideA)
                        &&(j->getPM().getSide() == JPetPM::SideB)
                    ){
                        recoSignalA.setRawSignal(*i);
                        recoSignalB.setRawSignal(*j);
                    }
                    else if(
                        (j->getPM().getSide() == JPetPM::SideA)
                        &&(i->getPM().getSide() == JPetPM::SideB)
                    ){
                        recoSignalA.setRawSignal(*j);
                        recoSignalB.setRawSignal(*i);
                    }
                    else {
                        // if two hits on the same side, ignore
                        WARNING("TWO hits on the same scintillator side we ignore it");
                        continue;
                    }

                    physSignalA.setRecoSignal(recoSignalA);
                    physSignalB.setRecoSignal(recoSignalB);

                    auto leadTimesA = recoSignalA.getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
                    auto leadTimesB = recoSignalB.getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

                    double timesSumA = 0;
                    double timesSumB = 0;

                    for(int itr = 1; itr <= 4; itr++){

                        timesSumA += leadTimesA[itr];
                        timesSumB += leadTimesB[itr];

                    }

                    physSignalA.setTime(timesSumA/4);
                    physSignalB.setTime(timesSumB/4);

                    if ( (physSignalA.getTime() - physSignalB.getTime()) < 10000 /*ps*/){
                        JPetHit hit;
                        hit.setSignalA(physSignalA);
                        hit.setSignalB(physSignalB);
                        hit.setTime( (physSignalB.getTime() + physSignalB.getTime())/2 );
                        hit.setScintillator(i->getPM().getScin());
                        hit.setBarrelSlot(i->getPM().getScin().getBarrelSlot());


                            // cout << recoSignalA.getRawSignal().getTOTsVsThresholdNumber().at(3) - recoSignalA.getRawSignal().getTOTsVsThresholdNumber().at(4)  << endl;


                        hits.push_back(hit);
                    }

                }
                getStatistics().getCounter("No. found hits")++;
            }
        }
    }
    return hits;
}

void ModuleC::terminate(){
    saveHits(createHits(fSignals)); //if there is something left
    INFO( Form("From %d initial signals %d hits were paired.",
           static_cast<int>(getStatistics().getCounter("No. initial signals")),
           static_cast<int>(getStatistics().getCounter("No. found hits")) )
    );
}


void ModuleC::saveHits(const vector<JPetHit>&hits){
    assert(fWriter);
    auto sorted = hits;
    sort(sorted.begin(), sorted.end(), [](const JPetHit & hit1, const JPetHit & hit2){return hit1.getTime() < hit2.getTime();});

    for (auto hit : sorted){
        // here one can impose any conditions on hits that should be
        // saved or skipped
        // for now, all hits are written to the output file
        // without checking anything
        fWriter->write(hit);
    }
}
void ModuleC::setWriter(JPetWriter* writer){fWriter =writer;}
