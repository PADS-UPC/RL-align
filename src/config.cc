//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2019  Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@cs.upc.es)
//             Computer Science Department
//             Omega-320 - Campus Nord UPC
//             C/ Jordi Girona 31
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "config.h"
#include "traces.h"

#define MOD_TRACENAME "CONFIG"
#define MOD_TRACECODE CFG_TRACE

using namespace std;

///////////////////////////////////////////////////////
/// empty constuctor

config::config() {
}

///////////////////////////////////////////////////////
/// read command line arguments and configuration file 

config::config(const string &fconfig) {

  ifstream scfg;
  scfg.open(fconfig);
  if (scfg.fail()) { ERROR_CRASH("Error opening file '" << fconfig << "'"); }
  
  string line;
  while (getline(scfg,line)) {
    if (line.empty() or line[0]=='#') continue;
    
    istringstream sin; sin.str(line);
    string key; string val;
    sin >> key >> val;      
    
    if (key == "DummyInitialWeight") DUMMY_INITIAL_WEIGHT=std::stod(val);
    else if (key == "DummyCompatibility") DUMMY_COMPAT=std::stod(val);
    else if (key == "ExclusiveCompatibility") EXCLUSIVE_COMPAT=std::stod(val);
    else if (key == "CrossCompatibility") CROSS_COMPAT=std::stod(val);
    else if (key == "RepeatCompatibility") REPEAT_COMPAT=std::stod(val);
    else if (key == "MaximumDistance") MAX_DIST = std::stoi(val);
    else if (key == "OrderCompatibility") {
      ORDER_COMPAT=std::stod(val);
      string type;
      sin >> type;
      ORDER_PROGRESSIVE = (type=="1/dist");
    }
    else if (key == "ParallelCompatibility") {
      PARALLEL_COMPAT=std::stod(val);
      string type;
      sin >> type;
      PARALLEL_PROGRESSIVE = (type=="1/dist");
    }
 
    else if (key == "RL_MaxIter") MAX_ITER = std::stoi(val);
    else if (key == "RL_ScaleFactor") SCALE_FACTOR = std::stod(val);
    else if (key == "RL_Epsilon") EPSILON = std::stod(val);

    else if (key == "AddIFS") ADD_IFS = (val!="false");
    else if (key == "AddLOOPS") ADD_LOOPS = (val!="false");

    else { WARNING("Ignoring unexpected key '" << key << "' in file '" << fconfig << "'."); }     
  }

  TRACE(1,"Read Configuration");
  TRACE(2,"  DummyInitialWeight = " << DUMMY_INITIAL_WEIGHT);
  TRACE(2,"  DummyCompatibility = " << DUMMY_COMPAT);
  TRACE(2,"  ExclusiveCompatibility = " << EXCLUSIVE_COMPAT);
  TRACE(2,"  CrossCompatibility = " << CROSS_COMPAT);
  TRACE(2,"  RepeatCompatibility = " <<  REPEAT_COMPAT);
  TRACE(2,"  OrderCompatibility = " << ORDER_COMPAT << " progressive:" << ORDER_PROGRESSIVE);
  TRACE(2,"  ParallelCompatibility = " << PARALLEL_COMPAT << " progressive:" << PARALLEL_PROGRESSIVE);
  TRACE(2,"  MaximumDistance = " << MAX_DIST);
  TRACE(2,"  AddIFS = " << ADD_IFS);
  TRACE(2,"  AddLOOPS = " << ADD_LOOPS);

  scfg.close();
}



config::~config() {}

