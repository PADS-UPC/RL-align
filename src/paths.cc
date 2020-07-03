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
#define MOD_TRACENAME "PATHS"
#define MOD_TRACECODE MAIN_TRACE

using namespace std;


bool belongs(const string &elm, const list<string> &elems) {
  for (auto e : elems)
    if (e==elm) return true;
  return false;  
}

void merge(list<string> &path, const list<string> &elems) {

  string sp; for (auto i : path) sp += " "+i;
  string se; for (auto i : elems) se += " "+i;

  list<string>::iterator p1 = path.begin();
  list<string>::const_iterator e1 = elems.begin();
  while (p1!=path.end() and e1!=elems.end() and *p1 == *e1) { ++p1; ++e1; }

  path.insert(p1, e1, elems.end());
  string sr; for (auto i : path) sr += " "+i;
  TRACE(3," [merge]  path=["<<sp<<"]   elem=["<<se<<"]   mergin=["<<sr<<"]");
}


// check if the path pik+pkj includes a parallel section. If it does, it must be complete.

bool valid_path(const string &i, map<string,list<string>>::iterator pik, map<string,list<string>>::iterator pkj, const map<string,string> &parallels, const map<string,list<string>> &paths) {

  list<string> s;
  s.push_back(i);
  s.insert(s.end(), pik->second.begin(), pik->second.end());
  s.insert(s.end(), pkj->second.begin(), pkj->second.end());

  bool ok = true;
  for (auto e = s.begin(); e!=s.end() and ok; ++e) {
    auto p = parallels.find(*e);
    if (p != parallels.end()) {
      string split = p->first;
      string join = p->second;
      size_t n = 0;
      auto f = e;
      while (f!=s.end() and *f!=join) {
        ++f;
        ++n;
      }
      
      // parallel limits where there, but middle was not complete, do not use this path.
      if (f!=s.end() and n < paths.find(split+":::"+join)->second.size()) 
           return false;
    }
  }
  
  return true;
}

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

void init_path_matrix(graph &g, map<string,list<string>> &paths, map<string,string> &parallels) {

  TRACE(1,"Init matrix");
  list<string> nodes = g.get_nodes_by_id();  
  for (auto n : nodes) {
    TRACE(1,"Init node "<<n);
    // init path matrix with direct edges
    set<string> succ = g.get_out_edges(n);
    for (auto s : succ) {
      list<string> p;
      p.push_back(s);
      paths[n+":::"+s] = p;
    }

    // node to self, cost zero
    list<string> p;
    paths[n+":::"+n] = p;

    // for parallel splits, compute path to matching parallel join
    if (g.is_parallel_split(n)) { 
      TRACE(2," is parallel split "<<n);
      string s = find_matching_join(g, n);
      if (s=="") { ERROR_CRASH("Couldn't find matching join for "<<n); }
      TRACE(2," found join at "<<s);

      list<string> p;
      // use A* to find shortest path to matching join
      g.BFS_LIMIT = 2000;
      bool found = g.find_path(g.get_out_edges(n), s, p);
      if (not found) {
        WARNING("Path not found from "<<n<<" to "<<s<<". Using simulation");

        // taking too long for A*, sample a number of random paths to matching join and select shortest.
        g.NUM_SAMPLE_PATHS = 200;
        p = g.find_path_by_sampling(n,s);
        if (p.empty()) {
          ERROR_CRASH("Simulation could not find a path from "<<n<<" to "<<s);
        }
      }

      TRACE(2,"PATH "<<n<<":"<<s<<"="<<list2string(p));

      paths[n+":::"+s] = p;
      parallels[n] = s;
    }
  }
}
  
//////////////////////////////////////////////////////
/// compute all distances using Floyd variant.

void floyd(const graph &g, map<string,list<string>> &paths, const map<string,string> &parallels) {
  TRACE(1,"Begin Floyd");
  // adapted floyd algorithm
  list<string> nodes = g.get_nodes_by_id();  
  for (auto k : nodes) {
    for (auto i : nodes) {
      // find path from i to k. If no path from i to k, skip
      auto pik = paths.find(i+":::"+k);
      if (pik == paths.end()) continue;

      for (auto j : nodes) {
        // find path from k to j. If no path from k to j, skip
        auto pkj = paths.find(k+":::"+j);
        if (pkj == paths.end()) continue;

        // if i-j is a complete parallel block, do not update cost. (this is the only adaptation needed)
        auto par = parallels.find(i);
        if (par!=parallels.end() and par->second==j) continue;
        
        // pik and pkj exist.  Check whether the composed path pik+pkj is valid
        if (valid_path(i,pik,pkj,parallels,paths)) {
          // find current path from i to j
          auto pij = paths.find(i+":::"+j);
          // if path pik+pkj is better than pij, update path
          if (pij == paths.end() or pij->second.size() > pik->second.size()+pkj->second.size()) {
            list<string> p;
            p.insert(p.end(), pik->second.begin(), pik->second.end());
            p.insert(p.end(), pkj->second.begin(), pkj->second.end());
            paths[i+":::"+j] = p;
          }
        }
      }
    }
  }
  TRACE(1,"End Floyd");
}

void fix_self_paths(const graph &g, map<string,list<string>> &paths, const map<string,string> &parallels) {

  list<string> nodes = g.get_nodes_by_id();  
  for (auto i : nodes) {
    bool found = false;
    if (g.is_parallel_split(i)){
      // if it is a parallel split, the best path i->i is running the whole parallel
      // and then going from the join to the split again
      string s = parallels.find(i)->second;
      auto psi= paths.find(s+":::"+i);
      if (psi!=paths.end()) {
        list<string> p;
        auto pis = paths.find(i+":::"+s);
        p.insert(p.end(), pis->second.begin(), pis->second.end());
        p.insert(p.end(), psi->second.begin(), psi->second.end());
        paths[i+":::"+i] = p;
        found = true;
      }
    }
    else {
      // not a parallel split. The best path to i->i is the best path from any successor of i
      set<string> succ = g.get_out_edges(i);
      size_t min = g.get_num_nodes()*2;
      string best;
      for (auto s : succ) {
        auto psi = paths.find(s+":::"+i);
        if (psi!=paths.end() and psi->second.size()<min) {
          min = psi->second.size();
          best = s;
        }
      }
      if (best!="") {
        list<string> p = paths.find(best+":::"+i)->second;
        p.push_front(best);
        paths[i+":::"+i] = p;
        found = true;
      }
    }
    if (not found) paths.erase(i+":::"+i);
  }
}

//////////////////////////////////////////////////////                            
/// print resulting path matrix                                                   

void output_path_matrix(ostream &sout, const graph &g, const map<string,list<string>> &paths) {

  list<string> nodes = g.get_nodes_by_id();
  for (auto i : nodes) {
    for (auto j : nodes) {
      int nn=0;
      string s = "";
      auto pij = paths.find(i+":::"+j);
      if (pij != paths.end()) {
        for (auto p : pij->second) {
            s +=  " " + p;
            ++nn;
        }
        // remove last node in the sequence (just a repetition of targ)           

        auto k = s.rfind(" "+j);
        s = s.substr(0,k);
      }

      sout << "PATH " << i << " " << j << " " << nn-1 << s << endl;
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
  map<string,list<string>> paths;
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


