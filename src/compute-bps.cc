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
#include <iostream>
#include <cmath>
#include <climits>
#include <algorithm>

#include "bp.h"
#include "graph.h"
#include "config.h"
#include "traces.h"
#include "util.h"
#define MOD_TRACENAME "COMPUTE_BP"
#define MOD_TRACECODE MAIN_TRACE

using namespace std;


//////////////////////////////////////////////////////
/// Initial BP, straighforward from paths

void init_bp(const graph &g, behavioral_profile &bp) {

  TRACE(2,"Fill initial BP using paths");
  list<string> nodes = g.get_nodes_by_id();
  for (auto i : nodes) {
    for (auto j : nodes) {
      // skip dummy node
      if (i==graph::DUMMY or j==graph::DUMMY) continue;
      // do only one direction, the other is simmetrical.
      if (i>j) continue;

      // check existence of paths i->j and j->i
      bool Epij = g.path_exists(i,j);
      bool Epji = g.path_exists(j,i);
      behavioral_profile::relType rel;
      if (Epij and Epji) // both paths found, parallel
        rel = behavioral_profile::INTERLEAVED;
      else if (Epij and not Epji) // only i->j found, precedes
        rel = behavioral_profile::PRECEDES;
      else if (not Epij and Epji) // only i<-j found, follows
        rel = behavioral_profile::FOLLOWS;
      else
        // otherwise, they are exclusive (unless they are in a parallel section, as checked below)
        rel = behavioral_profile::EXCLUSIVE;

      bp.add_relation(i,j,rel);
      bp.add_relation(j,i,bp.inverse(rel));
    }
  }
}

/// interleave nodes in two branches getting out of the same transition

void interleave_branches(const vector<list<string>> &branches, int i, int j, behavioral_profile &bp) {

  TRACE(4,"Branch "<<i<<" vs branch "<<j);

  // find first common node (both branches may join before the end of the region)
  string fcn = first_common_element(branches[i],branches[j]);
  if (fcn == "") {
    ERROR_CRASH("No common element found for branch "<<i<<" vs branch "<<j);
  }
  TRACE(4,"First common element is "<<fcn);

  auto last_bi = find(branches[i].begin(), branches[i].end(), fcn);
  auto last_bj = find(branches[j].begin(), branches[j].end(), fcn);
  
  // mark all nodes in branch i as parallel to all nodes in branch j (up to fcn in both branches)
  
  for (auto n1 = branches[i].begin(); n1!=last_bi; ++n1) {
    for (auto n2 = branches[j].begin(); n2!=last_bj; ++n2) {
      TRACE(5,"Node "<<*n1<<" (branch "<<i<<") - node "<<*n2<<" (branch "<<j<<")");
      behavioral_profile::relType rel = bp.get_relation(*n1,*n2);
      if (rel == behavioral_profile::EXCLUSIVE) {
        bp.add_relation(*n1,*n2,behavioral_profile::INTERLEAVED);
        bp.add_relation(*n2,*n1,behavioral_profile::INTERLEAVED);
      }
      else if (rel == behavioral_profile::INTERLEAVED) {
        // should not happen, but if it does, it is harmless
        TRACE(5,"Already parallel, skipping");
      }
      else {
        // should not happen, the net is probably not well-structured.
        ERROR_CRASH("Unexpected relation in parallel branches "<<*n1<<" "<<bp.get_rel_name(rel)<<" "<<*n2);
      }
    }
  }
}



/// fix paths in parallel regions

void fix_parallels(const graph &g, behavioral_profile &bp) {

  TRACE(2,"Fix paths in parallel regions");

  /// for each parallel region that is not in a loop,
  /// change the nodes in different branches in it from exclusive to parallel
  for (auto p : g.get_parallels()) {

    TRACE(3,"checking parallel region "<<p.first<<" "<<p.second);

    // if split and join are INTERLEAVED, the parallel section is in a loop, so
    // all their branches are also in the loop and already INTERLEAVED.
    if (bp.get_relation(p.first, p.second)==behavioral_profile::INTERLEAVED) {
      TRACE(3,"already parallel, skipping");
      continue;
    }

    // split and join are not already INTERLEAVED, but nodes in different branches
    // leaving split should be.

    // get all branches leaving split
    set<string> outs = g.get_out_edges(p.first);
    vector<list<string>> branches;
    int b = 0;

    for (auto succ : outs) {
      TRACE(3,"branch "<<succ<<" " <<p.second);
      list<string> branch;
      g.get_branch_nodes(succ, p.second, branch);
      TRACE(2,"    branch " << b << " {" << list2string(branch) <<"}");
      branches.push_back(branch);
      b++;
    }

    // mark nodes in different branches as INTERLEAVED.
    for (size_t i=0; i<branches.size()-1; ++i) 
      for (size_t j=i+1; j<branches.size(); ++j) 
        interleave_branches(branches, i, j, bp);
  }
}



//////////////////////////////////////////////////////
/// print resulting path matrix

behavioral_profile compute_BP(const graph &g) {

  behavioral_profile bp;

  // Initial BP, straighforward from paths
  init_bp(g, bp);

  // fix relations for nodes in the same parallel region
  fix_parallels(g, bp);

  // clean BP 
  list<string> nodes = g.get_nodes_by_id();
  for (auto i : nodes) {
    for (auto j : nodes) {
      if (g.get_node(i).type==node::PLACE or g.get_node(j).type==node::PLACE) {
        // remove relations not involving two transitions
        bp.remove_relation(i,j);
        bp.remove_relation(j,i);
      }
      else if (i>j)
        // keep only relations in one direction
        bp.remove_relation(i,j);
    }
  }

  // final resulting BP
  return bp;
}

/// ===========================
/// ========= MAIN ============
/// ===========================

int main(int argc, char *argv[]) {
  
  if (argc<4) {
    ERROR_CRASH("Usage: " << argv[0] << " model.pnml (original|unfolding) ifs loops [tracelevel]");
  }


  graph::NetVariant which = (string(argv[2]) == "original" ? graph::ORIGINAL : graph::UNFOLDING);
  bool ifs = string(argv[3]) != "false";
  bool loops = string(argv[4]) != "false";

  traces::set_tracing(argc>5 ? string(argv[5]) : "");

  // check PN file name
  string pnfile = string(argv[1]);
  size_t p = pnfile.find(".bp.pnml");
  if (p==string::npos) {ERROR_CRASH("Input file " + pnfile + " should be a .bp.pnml file");}
  // load petri net from XML file
  graph g(pnfile, which, ifs, loops);  
  // add "dummy" node (needed later by the aligning RL algorithm)
  if (which==graph::UNFOLDING)
    g.add_node(node(node::TRANSITION, graph::DUMMY, graph::DUMMY)); 

  // get name for corresponding path file and open it.
  string basename = pnfile.substr(0,p) + "." + (ifs?"t":"f") + (loops?"t":"f");
  string pathfile = basename + ".path";
  g.load_paths(pathfile);
     
  // compute BP using path info
  TRACE(1,"Computing BP");
  behavioral_profile bp = compute_BP(g);
  // print resulting BP
  cout << bp.dump();

}


