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


  graph g(argv[1], which_net, string(argv[3])=="true", string(argv[4])=="true"); // load XML model

  cout << g.dump();
}


