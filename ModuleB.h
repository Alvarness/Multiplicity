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
 *  @file ModuleB.h
 */

#ifndef MODULEB_H
#define MODULEB_H

#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include "LargeBarrelMapping.h"

class JPetWriter;

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

class ModuleB: public JPetTask
{

    public:
        ModuleB(const char * name, const char * description);
        virtual ~ModuleB();
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;
        virtual void setParamManager(JPetParamManager* paramManager)override;
        const JPetParamBank& getParamBank()const;

    protected:
        void saveRawSignal( JPetRawSignal sig);
        JPetWriter* fWriter;
        JPetParamManager* fParamManager;
        LargeBarrelMapping fBarrelMap;
};

#endif /*  !MODULEB_H */
