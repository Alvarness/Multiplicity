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
 *  @file main.cpp
 */

#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>
#include <JPetTaskLoader/JPetTaskLoader.h>
#include "ModuleA.h"
#include "ModuleB.h"
#include "ModuleC.h"
#include "ModuleD.h"

using namespace std;

int main(int argc, char* argv[]) {
    DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);

  // Here create all analysis modules to be used:

  manager.registerTask([](){
      return new JPetTaskLoader("hld", "tslot.raw",
                  new ModuleA("Module: Unp to TSlot Raw",
                        "Process unpacked HLD file into a tree of JPetTSlot objects"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("tslot.raw", "raw.sig",
                new ModuleB("Module: assemble signals",
                       "Assemble raw PMT signals from time window data"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("raw.sig", "phys.hit",
                new ModuleC("Module: Pair signals",
                      "Create hits from pairs of PMT signals"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("phys.hit", "phys.hit.means",
                new ModuleD("Module D: Make histograms for hits",
                      "Only make timeDiff histos and produce mean timeDiff value for each threshold and slot to be used by the next module"));
    });

  manager.run();
}
