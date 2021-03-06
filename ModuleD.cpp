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
 *  @file ModuleD.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "ModuleD.h"
using namespace std;
ModuleD::ModuleD(const char * name, const char * description):JPetTask(name, description){}

void ModuleD::init(const JPetTaskInterface::Options& opts){
    fBarrelMap.buildMappings(getParamBank());

    getStatistics().createHistogram( new TH1F("timeDifferenceLayer1", "Layer 3", 2*pow(10,6), -pow(10,9), pow(10,9)) );
    getStatistics().createHistogram( new TH1F("timeDifferenceLayer2", "Layer 2", 2*pow(10,6), -pow(10,9), pow(10,9)) );
    getStatistics().createHistogram( new TH1F("timeDifferenceLayer3", "Layer 3", 2*pow(10,6), -pow(10,9), pow(10,9)) );
    getStatistics().createHistogram( new TH1F("timeDifference", "All hits", 2*pow(10,6), -pow(10,9), pow(10,9)) );

    // create histograms for time differences at each slot and each threshold
    // for(auto & scin : getParamBank().getScintillators()){
    //     for (int thr=1;thr<=4;thr++){
    //         const char * histo_name = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_");
    //         getStatistics().createHistogram( new TH1F(histo_name, histo_name, 2000, -20., 20.) );
    //     }
    // }
    // // create histograms for time diffrerence vs slot ID
    // for(auto & layer : getParamBank().getLayers()){
    //     for (int thr=1;thr<=4;thr++){
    //         const char * histo_name = Form("TimeDiffVsID_layer_%d_thr_%d", fBarrelMap.getLayerNumber(*layer.second), thr);
    //         const char * histo_titile = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name);
    //         int n_slots_in_layer = fBarrelMap.getNumberOfSlots(*layer.second);
    //         getStatistics().createHistogram( new TH2F(histo_name, histo_titile, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
    //                               120, -20., 20.) );
    //     }
    // }
}

void ModuleD::exec(){
    //getting the data from event in propriate format

    if(auto currHit=dynamic_cast<const JPetHit*const>(getEvent())){
        if (fHits.empty()) {
            fHits.push_back(*currHit);
        }

        else {
            if (fHits[0].getTimeWindowIndex() == currHit->getSignalB().getTimeWindowIndex()) {
                fHits.push_back(*currHit);
            }
            else {
                // sort(fHits.begin(), fHits.end(), [](const JPetHit & hit1, const JPetHit & hit2){return hit1.getTime() < hit2.getTime();});
                fillTimeDiff(fHits);
                fHits.clear();
                fHits.push_back(*currHit);
            }
        }
    }
}


void ModuleD::terminate(){
    // save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
    // so that they are available to the consecutive modules
    getAuxilliaryData().createMap("timeDiffAB mean values");

    // for(auto & slot : getParamBank().getBarrelSlots()){
    //     for (int thr=1;thr<=4;thr++){
    //         const char * histo_name = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_");
    //         double mean = getStatistics().getHisto1D(histo_name).GetMean();
    //         getAuxilliaryData().setValue("timeDiffAB mean values", histo_name, mean);
    //     }
    // }


}

void ModuleD::fillTimeDiff(const vector<JPetHit>& hits){

    vector<JPetHit> layer1, layer2, layer3;

    for(int i=0; i<hits.size()-1; i++){
        getStatistics().getHisto1D("timeDifference").Fill(hits[i+1].getTime() - hits[i].getTime());
    }

    for (auto hit : hits) {
        int layerID = hit.getBarrelSlot().getLayer().getId();

        if(layerID == 1) layer1.push_back(hit);
        else if(layerID == 2) layer2.push_back(hit);
        else if(layerID == 3) layer3.push_back(hit);
    }

    if(layer1.size() > 1){
    for(int i = 0; i < layer1.size() - 1; i++){

        getStatistics().getHisto1D("timeDifferenceLayer1").Fill(layer1[i+1].getTime() - layer1[i].getTime());
    }
    }
    if(layer2.size() > 1){
    for(int i = 0; i < layer2.size() - 1; i++){

        getStatistics().getHisto1D("timeDifferenceLayer2").Fill(layer2[i+1].getTime() - layer2[i].getTime());
    }
    }
    if(layer3.size() > 1){
    for(int i = 0; i < layer3.size() - 1; i++){

        getStatistics().getHisto1D("timeDifferenceLayer3").Fill(layer3[i+1].getTime() - layer3[i].getTime());
    }
    }

}


void ModuleD::fillHistosForHit(const JPetHit & hit){
    auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
    auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

    for(auto & thr_time_pair : lead_times_A){
        int thr = thr_time_pair.first;
        if( lead_times_B.count(thr) > 0 ){ // if there was leading time at the same threshold at opposite side
            double timeDiffAB = lead_times_A[thr] - lead_times_B[thr];
            timeDiffAB /= 1000.; // we want the plots in ns instead of ps
            // fill the appropriate histogram
            const char * histo_name = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_");
            getStatistics().getHisto1D(histo_name).Fill( timeDiffAB );
            // fill the timeDiffAB vs slot ID histogram
            int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
            int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
            getStatistics().getHisto2D(Form("TimeDiffVsID_layer_%d_thr_%d", layer_number, thr)).Fill( slot_number,
                                                          timeDiffAB);
        }
    }


}

const char * ModuleD::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){
    int slot_number = fBarrelMap.getSlotNumber(slot);
    int layer_number = fBarrelMap.getLayerNumber(slot.getLayer());
    return Form("%slayer_%d_slot_%d_thr_%d",
        prefix,
        layer_number,
        slot_number,
        threshold
    );
}
void ModuleD::setWriter(JPetWriter* writer){fWriter =writer;}

