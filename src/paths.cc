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
  TRACE(2," [merge]  path=["<<sp<<"]   elem=["<<se<<"]   mergin=["<<sr<<"]");
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
    TRACE(2,"[required]    parallel at "<<targ<< " checking pred="<<ant);
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
    TRACE(2,"[required]    parallel at "<<targ<< " all branches check");
    ret.found = true;
    ret.elems.push_back(targ);
    ret.nested = 0;
  }
  else if (nok>0 and ret.nested>=depth) {
    TRACE(2,"[required]    parallel at "<<targ<< " one branches checks with depth limits");
    ret.found = true;
    ret.elems.push_back(targ);
  }
  else {
    TRACE(2,"[required]    parallel at "<<targ<< " no branches check");
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
    TRACE(2,"[required]    exclusive at "<<targ<< " checking pred="<<ant);
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
    TRACE(2,"[required]    exclusive at "<<targ<< " shortest branch found");
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

  map<string,list<string>> paths;
  string src, targ;
  list<string> seen;
  list<string> nodes = g.get_nodes_by_id();
  //while (cin >> src>> targ) {
  for (auto src : nodes) {
    // skip places unless initial and final
    //if (g.get_node(src).type == node::PLACE and g.get_node(src).id!=g.get_initial_node())
    //  continue;

    for (auto targ : nodes) {
      //if (g.get_node(targ).type == node::PLACE and g.get_node(targ).id!=g.get_final_node())
      //  continue;

      path r;
      TRACE(1, "MAIN ----  " << src << " -> " << targ);

      // src!=targ, normal case, find a path
      if (src != targ)         
        r = required(g, src, targ, seen, 0);
      
      // src == targ, find a path of length>0 (i.e. see if there is a loop)
      else if (g.get_node(src).type == node::TRANSITION and src!=graph::DUMMY) 
        r = parallel_join(g, src, targ, seen, false, 0);
      
      else // node.type == PLACE
        r = exclusive_join(g, src, targ, seen, false, 0);
      
      // print result
      int nn=0;
      string s = "";
      if (r.found) {
        for (auto p : r.elems) {
          //if (g.get_node(p).type==node::TRANSITION or g.get_node(p).id==g.get_final_node()) {
            s +=  " " + p;
            ++nn;
          //}
        }
        // remove last node in the sequence (just a repetition of targ)
        auto k = s.find(" "+targ);
        s = s.substr(0,k);
      }

      cout << src << " " << targ << " " << nn-1 << s << endl;
    }
  }

}


