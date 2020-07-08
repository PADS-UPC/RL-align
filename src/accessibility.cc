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
#include <fstream>
#include <cmath>
#include <climits>

#include "graph.h"
#include "config.h"
#include "traces.h"
#include "util.h"
#define MOD_TRACENAME "ACCESSIBLE"
#define MOD_TRACECODE MAIN_TRACE

using namespace std;


/// recursive auxiliary for find_matching_join, doing the actual work

string find_matching_join(const graph &g, const set<string> &open, set<string> visited) {

  set<string> ptr = g.possible_transitions(open);
  TRACE(3,"  open={" <<set2string(open)<<"}  ptr={"<<set2string(ptr)<<"}  visited={"<<set2string(visited)<<"}");

  // compute intersection of transitions accessble from all open places
  set<string> outs = g.get_out_edges(*(open.begin()));
  for (auto op : open)
    outs = intersection_set(outs,g.get_out_edges(op));

  // if the intersection of all open places is exactly one
  // transition, that is the join we were looking for.
  if (outs.size()==1) return *(outs.begin());
  // if no possible transitions, backtrace  
  else if (ptr.size()==0) return "";
  // if a loop is detected, backtrace
  else if (not intersection_set(open, visited).empty()) return "";
  // recurse into all possible transitions.
  else {   
    // fire all transitions that can be fired simultaneously (no conflicts involved)
    set<string> newopen = open;
    set<string> fired;
    for (auto p : open) {
      if (not g.is_exclusive_split(p)) {
        for (auto t : intersection_set(ptr,g.get_out_edges(p))) {
          newopen = g.fire_transition(newopen,t);
          fired.insert(t);
          visited.insert(p);
        }
      }
    }

    ptr = difference_set(ptr,fired);
    if (ptr.empty()) {
      // no conflicts, just continue from current situation
      string found = find_matching_join(g, newopen, visited);
      if (found!="") return found;
    }
    else {
      // remaining transitions are in conflict, use backtracking
      for (auto t : ptr) {
        string found = find_matching_join(g, g.fire_transition(newopen,t), union_set(visited, g.get_in_edges(t)));
        if (found!="") return found;
      }
    }
    
    return "";
  }
}


/// find matching join for given parallel split

string find_matching_join(const graph &g, const string &split) {
  set<string> visited;
  return find_matching_join(g, g.get_out_edges(split), visited);
}



//////////////////////////////////////////////////////
/// init matrix for Floyd

void init_path_matrix(graph &g, set<string> &paths, map<string,string> &parallels) {

  TRACE(1,"Init matrix");
  list<string> nodes = g.get_nodes_by_id();  
  for (auto n : nodes) {
    TRACE(1,"Init node "<<n);
    // init path matrix with direct edges
    set<string> succ = g.get_out_edges(n);
    for (auto s : succ) {
      paths.insert(n+":::"+s);
    }

    // node to self, 
    list<string> p;
    paths.insert(n+":::"+n);

    // for parallel splits, compute matching parallel join
    if (g.is_parallel_split(n)) { 
      TRACE(2," is parallel split "<<n);
      string s = find_matching_join(g, n);
      if (s=="") { ERROR_CRASH("Couldn't find matching join for "<<n); }
      TRACE(2," found join at "<<s);

      paths.insert(n+":::"+s);
      parallels[n] = s;
    }
  }
}
  
//////////////////////////////////////////////////////
/// compute all distances using Floyd variant.

void floyd(const graph &g, set<string> &paths, const map<string,string> &parallels) {
  TRACE(1,"Begin Floyd");
  // adapted floyd algorithm
  list<string> nodes = g.get_nodes_by_id();  
  for (auto k : nodes) {
    for (auto i : nodes) {
      // find path from i to k. If no path from i to k, skip
      auto pik = paths.find(i+":::"+k);
      if (pik == paths.end()) continue;

      for (auto j : nodes) {
        if (paths.find(i+":::"+j)!=paths.end()) continue;  // already accessible, skip
        
        // find path from k to j. If no path from k to j, skip
        auto pkj = paths.find(k+":::"+j);
        if (pkj == paths.end()) continue;

        // pik and pkj exist, thus pij too
        paths.insert(i+":::"+j);
      }
    }
  }
  TRACE(1,"End Floyd");
}

void fix_self_paths(const graph &g, set<string> &paths, const map<string,string> &parallels) {

  list<string> nodes = g.get_nodes_by_id();  
  for (auto i : nodes) {
    bool found = false;
    if (g.is_parallel_split(i)){
      // if it is a parallel split, the best path i->i is running the whole parallel
      // and then going from the join to the split again
      string s = parallels.find(i)->second;
      auto psi= paths.find(s+":::"+i);
      if (psi!=paths.end()) 
        found = true;
    }
    else {
      // not a parallel split. The best path to i->i is the best path from any successor of i
      set<string> succ = g.get_out_edges(i);
      for (auto s : succ) {
        auto psi = paths.find(s+":::"+i);
        if (psi != paths.end()) {
          found = true;
          break;
        }
      }
    }
    if (not found) paths.erase(i+":::"+i);
  }
}


//////////////////////////////////////////////////////                            
/// print resulting path matrix                                                   

void output_path_matrix(ostream &sout, const graph &g, const set<string> &paths) {

  list<string> nodes = g.get_nodes_by_id();
  for (auto i : nodes) {
    for (auto j : nodes) {
      int s = (paths.find(i+":::"+j) != paths.end() ? 1 : -1);
      sout << "PATH " << i << " " << j << " " << s << endl;
    }
  }
}


//////////////////////////////////////////////////////                            
/// print parallel regions

void output_parallels(ostream &sout, const map<string,string> par) {
  for (auto p : par)
    sout << "PARALLEL "<< p.first << " " << p.second << endl;
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
  
  graph g(argv[1], which, ifs, loops);  // load XML model
  if (which==graph::UNFOLDING)
    g.add_node(node(node::TRANSITION, graph::DUMMY, graph::DUMMY));  // add "dummy" node
  
  // Init cost matrix
  set<string> paths;
  map<string,string> parallels;
  init_path_matrix(g, paths, parallels);

  // compute all distances using Floyd variant.
  floyd(g, paths, parallels);

  // fix self-paths (we want paths from one note to itself to capture loops, if there are any)
  fix_self_paths(g, paths, parallels);
  
  TRACE(1,"Output paths");
  output_path_matrix(cout, g, paths);
  output_parallels(cout, parallels);
}


