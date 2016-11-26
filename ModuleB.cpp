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
 *  @file ModuleB.cpp
 */

#include <map>
#include <string>
#include <JPetWriter/JPetWriter.h>
#include "ModuleB.h"

using namespace std;

ModuleB::ModuleB(const char * name, const char * description):
JPetTask(name, description){}
ModuleB::~ModuleB(){}

void ModuleB::init(const JPetTaskInterface::Options& opts){
    getStatistics().createHistogram( new TH1F("Lead", "Points on leading edge", 7, -0.5, 6.7) );
    getStatistics().createHistogram( new TH1F("Trail", "Points on trailing edge", 7, -0.5, 6.7) );
    getStatistics().createHistogram( new TH1F("Lead+Trail", "Points on leading and trailing edge", 7, -0.5, 6.7) );

}

void ModuleB::exec(){
    //getting the data from event in propriate format
    if(auto timeWindow = dynamic_cast<const JPetTimeWindow*const>(getEvent())){

        map<int,JPetSigCh> leadSigChs;
        map<int,JPetSigCh> trailSigChs;
        map<int,JPetRawSignal> signals;

        const size_t nSigChs = timeWindow->getNumberOfSigCh();

        for (auto i = 0; i < nSigChs; i++) {

            JPetSigCh sigch = timeWindow->operator[](i);
            int daq_channel = sigch.getChannel();

            if( sigch.getType() == JPetSigCh::Leading )
                leadSigChs[ daq_channel ] = sigch;
            if( sigch.getType() == JPetSigCh::Trailing )
                trailSigChs[ daq_channel ] = sigch;
        }

        // iterate over the leading-edge SigChs
        for (auto & chSigPair : leadSigChs) {

            int daq_channel = chSigPair.first;

            if( trailSigChs.count(daq_channel) != 0 ){

                JPetSigCh & leadSigCh = chSigPair.second;
                JPetSigCh & trailSigCh = trailSigChs.at(daq_channel);

                // double tot = trailSigCh.getValue() - leadSigCh.getValue();

                if( leadSigCh.getPM() != trailSigCh.getPM() ){
                    ERROR("Signals from same channel point to different PMTs! Check the setup mapping!!!");
                }

                double pmt_id = trailSigCh.getPM().getID();
                signals[pmt_id].addPoint( leadSigCh );
                signals[pmt_id].addPoint( trailSigCh );
            }
        }

        for(auto & pmSignalPair : signals){

            auto & signal = pmSignalPair.second;

            signal.setTimeWindowIndex( timeWindow->getIndex() );

            const auto & pmt = getParamBank().getPM(pmSignalPair.first);

            signal.setPM(pmt);
            signal.setBarrelSlot(pmt.getBarrelSlot());

            fWriter->write(signal);

            getStatistics().getHisto1D("Lead").Fill(signal.getNumberOfLeadingEdgePoints());
            getStatistics().getHisto1D("Trail").Fill(signal.getNumberOfTrailingEdgePoints());
            if(signal.getNumberOfLeadingEdgePoints() == signal.getNumberOfTrailingEdgePoints()){
                getStatistics().getHisto1D("Lead+Trail").Fill(signal.getNumberOfTrailingEdgePoints());
            }
        }
    }
}
void ModuleB::terminate(){}
void ModuleB::saveRawSignal( JPetRawSignal sig){
    assert(fWriter);
    fWriter->write(sig);
}

void ModuleB::setWriter(JPetWriter* writer) {
    fWriter = writer;
}
void ModuleB::setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
}
const JPetParamBank& ModuleB::getParamBank() const {
    return fParamManager->getParamBank();
}
