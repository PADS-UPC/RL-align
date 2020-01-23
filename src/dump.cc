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

#include <iostream>
#include <cmath>
#include <climits>

#include "graph.h"
#include "config.h"
#include "traces.h"
#define MOD_TRACENAME "LOOPS"
#define MOD_TRACECODE MAIN_TRACE


using namespace std;


/// ===========================
/// ========= MAIN ============
/// ===========================

int main(int argc, char *argv[]) {

  if (argc<5) {
    ERROR_CRASH("Usage: " << argv[0] << " model (original|unfolding) addIfs addLoops" );
  }
  
  graph::NetVariant which_net = (string(argv[2]) == "original"
				 ? graph::ORIGINAL
				 : graph::UNFOLDING);


  //traces::set_tracing("3:0x3");
  graph g(argv[1], which_net, string(argv[3])=="true", string(argv[4])=="true"); // load XML model

  cout << g.dump();
}


