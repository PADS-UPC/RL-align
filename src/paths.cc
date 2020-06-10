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


// compute nodes needed to go from src to targ in g
struct path {
  bool found;  // whether requested path was found
  list<string> elems;  // graph elements forming the path
  int nested; // number of nested parallel blocks when src was found
};

// predeclaration
path required(const graph &g, string src, string targ, list<string> &seen, int depth);


// treat all antecessors of a parallel join and make sure all paths are satisfied

path parallel_join(const graph &g, string src, string targ, list<string> &seen, bool push, int depth) {
  path ret;
  ret.found = false;
  ret.nested = 0;
  int nok = 0;
  set<string> in = g.get_in_edges(targ);
  for (auto ant : in) {
    TRACE(3,"[required]    parallel at "<<targ<< " checking pred="<<ant);
    if (push) seen.push_back(targ);
    path r = required(g, src, ant, seen, depth);
    if (push) seen.pop_back();

    if (r.found) {
      merge(ret.elems, r.elems);  // current subpath works, merge with previous
      ret.nested = depth;
      ++nok;
    }
  }
  
  // if all subpaths work (or only one but inside depth), return true
  if (nok == int(in.size())) {
    TRACE(3,"[required]    parallel at "<<targ<< " all branches check");
    ret.found = true;
    ret.elems.push_back(targ);
    ret.nested = 0;
  }
  else if (nok>0 and ret.nested>=depth) {
    TRACE(3,"[required]    parallel at "<<targ<< " one branches checks with depth limits");
    ret.found = true;
    ret.elems.push_back(targ);
  }
  else {
    TRACE(3,"[required]    parallel at "<<targ<< " no branches check");
    ret.found = false;
  }
  return ret;
}

// treat all antecessors of an exclusive join and find shortest path, if any

path exclusive_join(const graph &g, string src, string targ, list<string> &seen, bool push, int depth) {
  path ret;
  ret.found = false;
  ret.nested = 0;
  int minlen = g.get_num_nodes()*2;// initialize min to a big number;
  set<string> in = g.get_in_edges(targ);
  for (auto ant : in) {
    TRACE(3,"[required]    exclusive at "<<targ<< " checking pred="<<ant);
    if (push) seen.push_back(targ);
    path r = required(g, src, ant, seen, depth);
    if (push) seen.pop_back();

    if (r.found) {
      int ps = r.elems.size();
      if (ps < minlen) {
        ret = r;
        minlen = ps;
      }
    }
  }
  
  if (ret.found) {
    TRACE(3,"[required]    exclusive at "<<targ<< " shortest branch found");
    ret.elems.push_back(targ);
  }
  return ret;        
}


// recursively find shortest path from src to targ.

path required(const graph &g, string src, string targ, list<string> &seen, int depth) {

  string ss; for (auto i : seen) ss += " "+i;
  TRACE(2, "[required] entering at " << targ << " looking back for " <<src << " seen=["<<ss<<"]");

  path ret;
  ret.nested = 0;
  
  if (src == targ) {
    ret.found = true;
    ret.nested = depth;
    return ret;
  }

  else {
    set<string> in = g.get_in_edges(targ);
    set<string> out = g.get_out_edges(targ);

    if (g.get_node(targ).type == node::TRANSITION) {
      // if parallel split and/or join, adjust depth level
      if (in.size()>1) ++depth;
      if (out.size()>1) --depth;
    }
    
    if (in.size()==0 or g.is_initial(targ) ) {
      TRACE(2,"[required] reached top - return false");
      // we reached the initial node, a node with no antecessors without finding src: There is no path.
      ret.found = false;
      return ret;
    }

    else if (belongs(targ,seen)) {
     TRACE(2,"[required] loop detected - return false" << (g.get_node(targ).type == node::TRANSITION));
     // we are in a node that was already visited
     ret.found =  false;
     return ret;
    }
    
    else if (in.size() == 1) {
      // only one antecessor, just add it to the solution
      string ant = *in.begin();
      seen.push_back(targ);
      ret = required(g, src, ant, seen, depth);
      seen.pop_back();
      if (ret.found) ret.elems.push_back(targ);
      return ret;
    }

    else if (g.get_node(targ).type == node::TRANSITION) {
      // A TRANSITION with more than one antecessor is a parallel join: all paths are required
      TRACE(2,"[required] parellel join detected at "<<targ);
      return parallel_join(g, src, targ, seen, true, depth);
    }

    else {
      // A PLACE with more than one antecessor is an exclusive join: select shortest path
      TRACE(2,"[required] exclusive deteted at "<< targ);
      return exclusive_join(g, src, targ, seen, true, depth);
    } 
  }
}


// check if the path pik+pkj includes a parallel section. If it does, it must be complete.

bool valid_path(const string &i, map<string,path>::iterator pik, map<string,path>::iterator pkj, const map<string,string> &parallels, const map<string,path> &paths) {

  list<string> s;
  s.push_back(i);
  s.insert(s.end(), pik->second.elems.begin(), pik->second.elems.end());
  s.insert(s.end(), pkj->second.elems.begin(), pkj->second.elems.end());

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
      if (f!=s.end() and n < paths.find(split+":::"+join)->second.elems.size()) 
           return false;
    }
  }
  
  return true;
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
  
  
  list<string> nodes = g.get_nodes_by_id();
  map<string,path> paths;
  map<string,string> parallels;
      
  for (auto n : nodes) {
    TRACE(1,"Init node "<<n);
    // init path matrix with direct edges
    set<string> succ = g.get_out_edges(n);
    for (auto s : succ) {
      path p;
      p.elems.push_back(s);
      paths[n+":::"+s] = p;
    }

    // node to self, cost zero
    path p;
    paths[n+":::"+n] = p;

    // for parallel splits, compute path to matching parallel join
    if (g.is_parallel_split(n)) { 
      TRACE(2," is parallel split "<<n);
      string s = g.find_matching_join(n);
      TRACE(2," found join at "<<s);

      // deprecated, used brute force, too slow in some cases.
      //list<string> seen;
      //path p = required(g, n, s, seen, 0);

      // now use A* to find shortest path to matching join
      path p;
      list<string> ls;
      p.found = g.find_path(g.get_out_edges(n), s, p.elems);
      if (not p.found) {
        WARNING("Path not found from "<<n<<" to "<<s);
      }
      else
        TRACE(2,"PATH "<<n<<":"<<s<<"="<<list2string(p.elems));

      paths[n+":::"+s] = p;
      parallels[n] = s;
    }
  }

  TRACE(1,"Begin Floyd");
  // adapted floyd algorithm
  for (auto k : nodes) {
    for (auto i : nodes) {
      for (auto j : nodes) {
        // if i-j is a complete parallel block, do not update cost. (this is the only adaptation needed)
        auto par = parallels.find(i);
        if (par!=parallels.end() and par->second==j) continue;

        // find current path from i to j
        auto pij = paths.find(i+":::"+j);
        // find paths from i to k, and from k to j
        auto pik = paths.find(i+":::"+k);
        auto pkj = paths.find(k+":::"+j);
        // if pik and pij exist and are better than pij, replace path
        if (pik != paths.end() and pkj != paths.end() and valid_path(i,pik,pkj,parallels,paths)) {
          if (pij == paths.end() or pij->second.elems.size() > pik->second.elems.size()+pkj->second.elems.size()) {
            path p;
            p.elems.insert(p.elems.end(), pik->second.elems.begin(), pik->second.elems.end());
            p.elems.insert(p.elems.end(), pkj->second.elems.begin(), pkj->second.elems.end());
            paths[i+":::"+j] = p;
          }
        }
      }
    }
  }
  TRACE(1,"End Floyd");

  // we want paths from one note to itself to capture loops, if there are any
  for (auto i : nodes) {
    bool found = false;
    if (g.is_parallel_split(i)){
      // if it is a parallel split, the best path i->i is running the whole parallel and then goind from the join to the split again
      string s = g.find_matching_join(i);
      auto psi= paths.find(s+":::"+i);
      if (psi!=paths.end()) {
        path p;
        auto pis = paths.find(i+":::"+s);
        p.elems.insert(p.elems.end(), pis->second.elems.begin(), pis->second.elems.end());
        p.elems.insert(p.elems.end(), psi->second.elems.begin(), psi->second.elems.end());
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
        if (psi!=paths.end() and psi->second.elems.size()<min) {
          min = psi->second.elems.size();
          best = s;
        }
      }
      if (best!="") {
        path p = paths.find(best+":::"+i)->second;
        p.elems.push_front(best);
        paths[i+":::"+i] = p;
        found = true;
      }
    }
    if (not found) paths.erase(i+":::"+i);
  }

  
  // print result
  for (auto i : nodes) {
    for (auto j : nodes) {
      int nn=0;
      string s = "";
      auto pij = paths.find(i+":::"+j);
      if (pij != paths.end()) {
        for (auto p : pij->second.elems) {
            s +=  " " + p;
            ++nn;
        }
        // remove last node in the sequence (just a repetition of targ)
        auto k = s.find(" "+j);
        s = s.substr(0,k);
      }

      cout << i << " " << j << " " << nn-1 << s << endl;
    }
  }

}


