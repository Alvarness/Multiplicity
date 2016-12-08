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
 *  @file ModuleD.h
 */

#ifndef ModuleD_H
#define ModuleD_H

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include "LargeBarrelMapping.h"

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

class ModuleD:public JPetTask{

    public:
        ModuleD(const char * name, const char * description);
        virtual ~ModuleD(){}
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;

    protected:
        const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
        void fillHistosForHit(const JPetHit & hit);
        void fillTimeDiff(const std::vector<JPetHit>& hits);
        JPetWriter* fWriter;
        LargeBarrelMapping fBarrelMap;
        double layer1 = -123456.7, layer2 = -123456.7, layer3 = -123456.7;
        std::vector<JPetHit> fHits;
        int multiplicity = 1;

};

#endif /*  !ModuleD_H */
