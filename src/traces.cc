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

#include "traces.h"

#define MOD_TRACENAME "TRACES"

int traces::Level = 0;
unsigned long traces::Module = 0;

void traces::set_tracing(const std::string &trace) {

  /// trace has format 99:0x99999
  /// number before the colon is the verbosity level.
  /// hex number after the colon is an hexmask indicating which modules are active.
  
  int verbose = 0;
  unsigned long modules = 0;
  if (not trace.empty()) {
    size_t p = trace.find(":");
    if (p==std::string::npos) {WARNING("Invalid trace value '"<<trace<<"' ignored.");}
    else {      
      verbose = stoi(trace.substr(0,p));
      modules = stoul(trace.substr(p+1),0,0);
    }
  }
  
  traces::Level = verbose;
  traces::Module = modules;
}


