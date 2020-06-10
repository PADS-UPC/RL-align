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
#include <sstream>
#include <climits>
#include <cmath>
#include <iterator>
#include <algorithm>

#include "graph.h"
#include "util.h"

/// Tracing and printing stuff
#include "traces.h"
#define MOD_TRACENAME "GRAPH"
#define MOD_TRACECODE GRAPH_TRACE

using namespace std;

const std::string graph::DUMMY = "_DUMMY_";
const int graph::BFS_LIMIT = 400;

//////////// Class node stores a graph node //////////////7

/// empty constructor

node::node() {}

/// constructor from data

node::node(NodeType t, const std::string &i, const std::string &n, bool init) {
  type = t;
  id = i;
  name = n;
  initial_marking = init;
}

/// destructor

node::~node() {}

//////////// Class graph stores nodes&edges of a graph and offers useful consulting methods //////////////

/// constructor

graph::graph() {}

/// constructor from a xml file (.pnml)

graph::graph(const string &fname, NetVariant which, bool addIFS, bool addLOOPS) {
    // open input file
    pugi::xml_document xmldoc;
    xmldoc.load_file(fname.c_str(), pugi::parse_default|pugi::parse_ws_pcdata);

    // get "page" node either under "original_net" or at top level (which is the unfolding)
    pugi::xml_node page = (which == ORIGINAL
			   ? xmldoc.child("pnml").child("net").child("original_net").child("net").child("page")
			   : xmldoc.child("pnml").child("net").child("page"));

    TRACE(6,"Loading nodes");
    load_nodes(page, "place", which);      // add "place" nodes to the graph
    load_nodes(page, "transition", which);  // add "transition" nodes to the graph

    TRACE(6,"Loading edges");
    // add edges to the graph
    pugi::xml_node arc = page.child("arc");
    while (arc) {
      add_edge(arc.attribute("source").value(), arc.attribute("target").value());
      arc = arc.next_sibling("arc");
    }
    
    if (which == ORIGINAL) {
      // locate initial and final nodes
      for (auto n : nodes_by_id) {
	if (get_in_edges(n.first).empty() and n.second.initial_marking) 
	  initial_nodes.insert(n.first);
	if (get_out_edges(n.first).empty())
	  final_nodes.insert(n.first);
      }
    }

    else { // which == UNFOLDING
      
      TRACE(6,"Rebuilding cut-offs");
      // find leave nodes, and if they have a repeated "original node", redirect the incoming edges
      list<pair<string,string>> to_be_replaced;
      for (auto i : nodes_by_id) {
	node &nd = i.second;
	if (not is_leaf(nd.id)) continue;  // not a leaf, skip

	TRACE(3,"checking for loops " << nd.id); 
	// It is a leaf. Locate other non-leaf nodes with the same name.
	list<string> ref = get_nodes_by_name(nd.name);

	list<string> rid;
	for (auto t : ref) {                
	  // we look for a non-leaf node with the same name (which is not the node itself)
	  if (t!=nd.id and not is_leaf(t))
	    rid.push_back(t);
	}

	if (rid.size() > 0) {
          TRACE(3,"   found " << rid.size() << " non-leaf nodes with same name than " << nd.id << " (" << nd.name << ")" ); 
          TRACE(3,"   [" << list2string(rid) << "]");

          string refid;
	  bool isloop = false;
          string log="IF";
          for (auto r : rid) {
            // for IF cutoffs, select any in the list (they should all have the same future)
            refid = r;
            if (accessible(r,nd.id)) {
              // if there is a path from rid to nd.id, adding the new edge would create a loop
              isloop = true;
              log = "LOOP";
              break;
            }
          }

          if ((isloop and addLOOPS) or (not isloop and addIFS)) {
            TRACE(3, "redirecting " << log << ": " << nd.id << " to " << refid);
            // remember that edges arriving to nd will be redirected to rid
            to_be_replaced.push_back(make_pair(nd.id,refid));
          }
          else {
            TRACE(3, "ignoring " << log << ": " << nd.id << " to " << refid);
          }
	}
      }

      // replace arcs arriving to nd with arcs to rid.
      TRACE(6,"Redirecting "<<to_be_replaced.size()<<" cut-off nodes.");
      for (auto p : to_be_replaced)
        replace_node(p.first,p.second);

      // Locate initial and final node
      for (auto n : nodes_by_id) {
	if (get_in_edges(n.first).empty()) {
          TRACE(6,"Adding initial node "<<n.first);
	  if (initial_nodes.empty()) initial_nodes.insert(n.first);
	  else { ERROR_CRASH("More than one initial node detected ("<< *initial_nodes.begin() <<", "<< n.first <<")."); }
	}
	if (get_out_edges(n.first).empty()) {
	  final_nodes.insert(n.first);
	}
      }

      if (initial_nodes.empty()) {
        WARNING("WARNING: No pure initial nodes detected. Defaulting to c0");
        if (nodes_by_id.find("c0")==nodes_by_id.end()) {
          ERROR_CRASH("No c0 node found");
        }
        else
          initial_nodes.insert("c0");         
      }
      
      // if more than one final node, unify them with the first in the list.
      TRACE(6,"Final nodes: [" << set2string(final_nodes) << "]");
      set<string> fn = final_nodes; // copy to avoid iterator break when deleting nodes
      auto f = fn.begin();
      string f1 = *f;
      ++f;
      while (f != fn.end()) {
        replace_node(*f, f1);
        ++f;
      }
      TRACE(6,"Unified "<<fn.size()<<" final nodes to ["<< set2string(final_nodes) << "]");
    }

}

/// destructor

graph::~graph() {}

/// extract nodes of given type from given XML tree, and load them into the graph

void graph::load_nodes(pugi::xml_node page, const std::string &type, NetVariant which) {
  node::NodeType t;
  if (type=="place") t = node::PLACE;
  else t = node::TRANSITION; // if (type=="transition") 

  // add nodes of type "key" to the graph
  pugi::xml_node n = page.child(type.c_str());
  while (n) {
    string name = (which==UNFOLDING
		   ? n.child("originalNode").child_value("text")
		   : n.child("name").child_value("text"));
    std::replace(name.begin(),name.end(),' ','_');
      
    node x(t,
           n.attribute("id").value(),
           name,
	   n.child("initialMarking")!=NULL);

    add_node(x);
      
    n = n.next_sibling(type.c_str());
  }
}

/// add given node to the graph

void graph::add_node(const node &n) {
  auto r = nodes_by_id.insert(make_pair(n.id,n));
  if (not r.second) {
    WARNING("Already existing node id=" << n.id << " not inserted.")
    return;
  }
  nodes_by_name.insert(make_pair(n.name,n.id));
}

/// remove node with given id

void graph::remove_node(const string &id) {
  string name = get_node(id).name;
  remove_from_multimap(nodes_by_name, name, id);
  nodes_by_id.erase(id);
  initial_nodes.erase(id); // (just in case it was there)
  final_nodes.erase(id);
}

/// get number of nodes in the graph

int graph::get_num_nodes() const {
  return nodes_by_id.size();
}

/// remove 'oldnode' from the graph, and make all incoming transitions go to newnode.
/// 'oldnode' must be a leaf.

void graph::replace_node(const string &oldnode, const string &newnode) {
  TRACE(6,"Redirecting "<<oldnode<<" to "<<newnode);
  set<string> pred = get_in_edges(oldnode);
  for (auto n : pred) {
    remove_edge(n, oldnode);
    add_edge(n, newnode);
  }
  remove_node(oldnode);    
}


/// add a directed edge from src to targ

void graph::add_edge(const string &src, const string &targ) {
  auto p = out_edges.equal_range(src);
  bool found=false;
  for (auto i=p.first; i!=p.second and not found; ++i)
    found = (i->second==targ);
  if (found) {
    WARNING("Already existing edge " << src << "->" << targ << " not inserted.");
  }
  else {
    out_edges.insert(make_pair(src,targ));
    in_edges.insert(make_pair(targ,src));
  }
}


/// remove directed edge src->targ

void graph::remove_edge(const string &src, const string &targ) {
    remove_from_multimap(out_edges,src,targ);
    remove_from_multimap(in_edges,targ,src);
}

/// compute distances

void graph::compute_distances() {
   
  TRACE(6,"Computing distances");
  
  // Initialize the solution matrix with adjacency information
  distances.clear();
  for (auto i : nodes_by_id) {
    for (auto j : nodes_by_id) {
      if (i.first==j.first)
        distances[i.first+" "+j.first] = 0;
      else if (connected(i.first,j.first))
        distances[i.first+" "+j.first] = 1;
      else
        distances[i.first+" "+j.first] = std::numeric_limits<double>::infinity();
    }
  }
  
  // For each vertice k between i and j, update distances if i-k-j path is shorter
  for (auto k : nodes_by_id) {
    // Pick all vertices as source one by one 
    for (auto i : nodes_by_id) {
      // Pick all vertices as destination for node i
      for (auto j : nodes_by_id) {
        // If vertex k is on the shortest path from i to j, then update the value of dist(i,j)
        double d = distance(i.first,k.first) + distance(k.first,j.first);
        if (d < distance(i.first,j.first)) 
           distances[i.first+" "+j.first] = d; 
      } 
    } 
  }  
}

/// save distances to given ostream

void graph::save_distances(ostream &sdist) const {
  for (auto d : distances)
    sdist << d.first << " " << d.second << endl;
}
   
/// load distances from given file

void graph::load_distances(const string &fname) {

  ifstream sdist;
  sdist.open(fname);
  if (sdist.fail()) { ERROR_CRASH("Error opening file '" << fname << "'"); }

  distances.clear();
  string n1,n2; double d;
  while (sdist >> n1 >> n2 >> d) 
    distances[n1+" "+n2] = d;
  
  sdist.close();
}
   
/// load distances and paths from given file

void graph::load_paths(const string &fname) {

  ifstream sdist;
  sdist.open(fname);
  if (sdist.fail()) { ERROR_CRASH("Error opening file '" << fname << "'"); }
  
  distances.clear();
  paths.clear();

  string line;
  while (getline(sdist,line)) {
    istringstream sin; sin.str(line);
    string n1,n2; double d;
    sin >> n1 >> n2 >> d; 
    distances[n1+" "+n2] = d;
    if (d >= 0) {
      list<string> p;
      string e;
      while (sin>>e) p.push_back(e);
      paths[n1+" "+n2] = p;
    }
  }
  sdist.close();
}
   


/// check for node existence

map<string,node>::const_iterator graph::check_node(const string &id) const {
  auto p = nodes_by_id.find(id);
  if (p==nodes_by_id.end()) {
    ERROR_CRASH("Unexisting node " << id << " requested.");
  }
  return p;
}

/// see if a node is a leave (no output edges)

bool graph::is_leaf(const string &id) const {
  const node& x = get_node(id);
  return (get_out_edges(x.id).empty());
}

/// get starting node for the PN

set<string> graph::get_initial_nodes() const {
  return initial_nodes;
}

/// get end node for the PN

set<string> graph::get_final_nodes() const {
  return final_nodes;
}

/// see if a node is initial

bool graph::is_initial(const string &id) const {
  return initial_nodes.find(id) != initial_nodes.end();
}

/// see if a node is initial

bool graph::is_final(const string &id) const {
  return final_nodes.find(id) != final_nodes.end();
}


/// get information for given node id

const node& graph::get_node(const string &id) const {
  return check_node(id)->second;
}


/// get ids of all nodes 

list<string> graph::get_nodes_by_id() const {
  list<string> res;
  for (auto i : nodes_by_id) 
    res.push_back(i.first);
  return res;
}

/// get ids of nodes with given name

list<string> graph::get_nodes_by_name(const string &name) const {
  list<string> nds;
  auto p = nodes_by_name.equal_range(name);
  for (auto i=p.first; i!=p.second; ++i)
     nds.push_back(i->second);
  return nds;
}

/// auxiliary to get In or out edges from given node

set<string> graph::get_edges(const string &id, const multimap<string,string> &edges) const {
  set<string> nds;
  auto p = edges.equal_range(id);
  for (auto i=p.first; i!=p.second; ++i)
     nds.insert(i->second);
  return nds;
}
  
/// get out edges from given node

set<string> graph::get_out_edges(const string &id) const {
  check_node(id);  // just make sure node exists
  return get_edges(id, out_edges);
}

/// get in edges to given node

set<string> graph::get_in_edges(const string &id) const {
  check_node(id);  // just make sure node exists
  return get_edges(id, in_edges);
}

/// check if a node is a parallel join

bool graph::is_parallel_join(const std::string &id) const {
  return (get_node(id).type == node::TRANSITION and
          get_in_edges(id).size() > 1);
}

/// check if a node is a parallel split

bool graph::is_parallel_split(const std::string &id) const {
  return (get_node(id).type == node::TRANSITION and
          get_out_edges(id).size() > 1);
}

/// find the parallel split matching given parallel join

string graph::find_matching_split(const string &id) const {

  if (not is_parallel_join(id)) {
    ERROR_CRASH("find_matching_split called for non-join node " << id);
  }

  int d = 0;
  string n = id;
  bool found = false;
  while (not found) {
    if (is_parallel_split(n)) {
      --d;       // it is a parallel split, reduce depth
      if (d==0)
        found = true; // we reached initial depth, it is the searched node
    }
    
    if (is_parallel_join(n)) 
      // parallel join, increase depth
      ++d; 
    
    else if (is_initial(n)) {
      WARNING("initial node reached when looking for parallel split");
      return "";
    }
    
    // if not found, take one more step backward.
    if (not found) n = *(get_in_edges(n).begin());
  }
  
  return n;
}


string graph::find_matching_join(const string &split, const string &id, int depth, set<string> &seen) const {

  TRACE(3,"looking for match for "<<split<<" at " << id);
  
  if (is_parallel_join(id) and id!=split) { // parallel join, decrease depth
    --depth;
    if (depth==0) return id; // we reached initial depth, it is the searched node
  }

  if (seen.count(id)>0) // this branch is a loop, go back
    return "";
      
  if (is_parallel_split(id)) {     // it is a parallel split, increase depth
    ++depth;   
  }  
  else if (is_final(id)) {
    WARNING("final node reached when looking for parallel join");
    return "";
  }
  
  // if not found, take one more step forward
  seen.insert(id);
  for (auto e : get_out_edges(id)) {
    string found = find_matching_join(split, e, depth, seen);
    if (found!="") return found;
  }
  seen.erase(id);

  return "";  
}

/// find the parallel join matching given parallel split


string graph::find_matching_join(const string &id) const {

  if (not is_parallel_split(id)) {
    ERROR_CRASH("find_matching_join called for non-split node " << id);
  }

  set<string> seen;
  return find_matching_join(id, id, 0, seen);
  
}
  /*
string graph::find_matching_join(const string &id) const {

  if (not is_parallel_split(id)) {
    ERROR_CRASH("find_matching_join called for non-split node " << id);
  }

  int d = 0;
  string n = id;
  bool found = false;
  while (not found) {
    TRACE(3,"looking for join for "<<id<<". now at "<<n);
    if (is_parallel_join(n)) {
      // parallel join, decrease depth
      --d;
      if (d==0)
        found = true; // we reached initial depth, it is the searched node
    }
    
    if (is_parallel_split(n)) {
      ++d;       // it is a parallel split, increase depth
    }
    
    else if (is_final(n)) {
      WARNING("final node reached when looking for parallel join");
      return "";
    }
    
    // if not found, take one more step forward
    if (not found) n = *(get_out_edges(n).begin());
  }
  
  return n;
}
  */
  
/// find out whether there is an edge src -> targ
 
bool graph::connected(const std::string &src, const std::string &targ) const {
  set<string> neighb = get_out_edges(src);
  for (auto n : neighb)
    if (n == targ) return true;
  return false;
}

/// find out whether src -> targ are connected. Only works on acyclic graphs (used only at load time, before adding loops)

bool graph::accessible(const string &src, const string &targ) const {
  if (connected(src,targ))
    return true;

  else {
    set<string> neighb = get_out_edges(src);
    for (auto n : neighb) {
      if (accessible(n,targ)) return true;
    }
    return false;
  }
}

/// get possible transitions from a PN configuration

set<string> graph::possible_transitions(const set<string> &open, string target) const {
  set<string> tr;
  // check for possible model or sync moves
  for (auto s : open) { // for each open place
    // if target was specified, but place "s" can not reach it, skip its transitions
    if (target!="" and distance(s,target)<0) continue; 

    set<string> succ = get_out_edges(s);
    for (auto t : succ) { // for each possible transition from s
      node tnode = get_node(t);
      // see if all markings to fire that transition are satisfied
      set<string> diff = difference_set(get_in_edges(t), open);
      if (diff.empty()) {
	// all required markings were in 'open', the transition can be fired
	tr.insert(t); 
      }
    }
  }
  return tr;
}

/// see if a PN configuration is final

bool graph::is_final(const set<string> &open) const {
  return includes(final_nodes.begin(), final_nodes.end(), open.begin(), open.end());
}


// fire a transition on given PN configuration and return new set of marked places
set<string> graph::fire_transition(const set<string> &open, const string &t) const {
  return union_set(difference_set(open, get_in_edges(t)),
                   get_out_edges(t));
}


// get minimum distance from any node in 'open' to 'target', or -1 if there is no path
int graph::shortest_distance(const set<string> &open, const string &target) const {
  int m = std::numeric_limits<int>::max();
  for (auto x : open) {
    int d = 0;
    if (x != target) d = distance(x,target);    
    if (d>=0 and d<m) m = d;
  }
  if (m == std::numeric_limits<int>::max()) return -1;
  else return m;
}

// get average distance from any node in 'open' to 'target', or -1 if there is no path
int graph::average_distance(const set<string> &open, const string &target) const {
  int m = 0;
  int s = 0;
  for (auto x : open) {
    int d = 0;
    if (x != target) d = distance(x,target);    
    if (d>=0) {
      s += d;
      ++m;
    }
  }
  return int(round(double(s)/m));
}

search_state::search_state(const set<string> &op, const list<string> &pth, int dist) { open_places = op; path = pth; distance=dist; }
search_state::~search_state() {}

bool search_state::operator<(const search_state &p) const {
  if (p.distance < 0) return true;
  else if (this->distance < 0 and p.distance >= 0) return false;
  else if (this->distance < p.distance) return true;
  else if (this->distance > p.distance) return false;
  else { // same distance, break tie
    return set2string(this->open_places) < set2string(p.open_places);
  }
}


// Perform BFS on petri net to find a path from current configuration ("open") to given transition (target)
bool graph::find_path(const set<string> &open, const string &target, list<string> &path) const {

  path.clear();
  set<search_state> pending;   // list of pending configurations to explore

  set<set<string>> seen;
  pending.insert(search_state(open,list<string>(),shortest_distance(open,target)));
  //pending.insert(search_state(open,list<string>(),average_distance(open,target)));
  int explored = 0;
  while (not pending.empty()) {

    search_state current = *pending.begin(); // get current state to explore
    pending.erase(pending.begin()); // remove from candidate list
    
    TRACE(3,"BFSearch path from configuration [" << set2string(current.open_places) << "] to node " << target);
    seen.insert(current.open_places);

    set<string> ptr = possible_transitions(current.open_places, target);
    TRACE(3,"   possible transitions: [" << set2string(ptr) << "]");

    if (current.open_places.find(target)!=current.open_places.end() or  // the target is a place and we reached it
        ptr.find(target) != ptr.end()) {                                // or the target is an enabled transition

      path = current.path;
      TRACE(3,"   Path found: [" << list2string(path) << "]");
      return true;
    }

    
    ++explored;
    if (explored>BFS_LIMIT and pending.size()>BFS_LIMIT) {
      // the alignment is probably wrong and there is no way to fix it,
      // or maybe the model is too big or complex to be worth solving it via A*
      TRACE(3,"   BFS Exploration is taking too long, giving up");
      return false;
    }
    
    if (current.distance < 0) {
      // if there is no conection from this configuration to target, do not expand it
      TRACE(3,"   No connection to target from here. Abandoning branch");
    }
    else {
      TRACE(3,"   Adding successor configurations to pending list");
      for (auto t : ptr) {
        // fire each transition and add resulting configuration for further exploration
        set<string> nextopen = fire_transition(current.open_places, t);
        TRACE(4,"      - possible sucessor firing " << t << ": " << set2string(nextopen));

        if (seen.find(nextopen) == seen.end()) {
          // if unseen configuration, expand.
          // extend partial path with fired transition and store pair (configuration, path) for BFS
          list<string> nextpath = current.path;
          nextpath.push_back(t);
          // heurisic for BFS: current path length+underestimation of remaining length.
          // (this BFS is actually an A*)
          // If this is too slow, try removing the first part of the sum and do a simple BFS.
          // Results will have higer cost, but search may be faster
	  int h = shortest_distance(nextopen,target);   // heuristic cost estimation
	  //int h = average_distance(nextopen,target);   // heuristic cost estimation
	  int g = nextpath.size();
	  int f; // A* cost function f = g + h
	  if (h<0) 
	    f = -1;   // no path from here, add negative cost so it is discarded when (or if) selected
	  else 
	    f = g + h;
	  
	  pending.insert(search_state(nextopen, nextpath, f));
	  TRACE(4,"        Adding "<< f << "("<<g<<"+"<<h<<")/" << list2string(nextpath) << "/" << set2string(nextopen));
	  TRACE(4,"        Pending size =" << pending.size());	  
        }
        else {
          // if configuration is already visited, skip
          TRACE(3,"   Skipping seen configuration [" << set2string(nextopen) << "]");
        }
      }
    }
    
    TRACE(3,"   Pending contains now "<< pending.size() << " configurations");
  }
  
  return false;
}


/// find out whether there is a path src -> targ. Requires that paths have been loaded

bool graph::path_exists(const string &src, const string &targ) const {
  return paths.find(src+" "+targ) != paths.end();
}

/// get distance between two nodes

double graph::distance(const string &id1, const string &id2) const {
  return distances.find(id1+" "+id2)->second;
}


/// get path between two nodes

list<string> graph::path(const string &id1, const string &id2) const {
  if (not path_exists(id1,id2)) {
    ERROR_CRASH("Path between unaccessible nodes " << id1 << " -> " << id2 << " requested.");
  }
  return paths.find(id1+" "+id2)->second;
}



/// utility: remove a pair (key,val) from given multimap

void graph::remove_from_multimap(multimap<string,string> &mmap, const string &key, const string &val) {
  auto p = mmap.equal_range(key);
  for (auto i = p.first; i!=p.second; ++i) {
    if (i->second==val) {
      mmap.erase(i);
      return;
    }
  }
}

/// utility: find out if there is a loop involving given node or any accessible from it.

bool graph::has_loops(const string &node, set<string> &seen) const {

  if (seen.count(node)!=0)
    return true;
  
  else {
    set<string> next = get_out_edges(node);
    seen.insert(node);
    for (auto n : next) {
      if (has_loops(n,seen))
        return true;
    }
    seen.erase(node);
    return false;
  }    
} 


/// Find out if a graph is acyclic

bool graph::is_acyclic() const {
  for (auto n : initial_nodes) {
    set<string> seen;
    if (has_loops(n,seen))
      return false;
  }
  return true;
}


/// Find out whether a trace fits the graph ===========================

bool graph::is_fitting(set<string> &open, const set<string> &final,
                       alignment::iterator from, alignment::iterator to,
                       alignment::iterator &curr,
                       bool skip_unexpected) const {

  TRACE(3,"checking is_fitting from "<< from->id <<" to " << to->id <<" open=["<< set2string(open) <<"]  final=["<< set2string(final) <<"]");

  curr = from;
  while (curr!=next(to)) {
    // ignore log moves
    if (curr->type != "[L]") {
      set<string> pred = get_in_edges(curr->id);
      // check if all required states are open
      if (includes(open.begin(), open.end(), pred.begin(), pred.end())) {
        open = difference_set(open, pred); // remove predecessors from open list
        set<string> succ = get_out_edges(curr->id);
        open = union_set(open,succ);       // add successors to open list
      }
      else if (not skip_unexpected) {
        stringstream q; for (auto x=curr; x!=next(to); ++x) q<<" "<<x->id;  
        TRACE(3,"Unexpected "<< curr->id <<" with open=["<< set2string(open) <<"]  seq=["<< q.str() <<"]");
        return false;
      }
      else {
        TRACE(3,"skipping unexpected "<< curr->id);
      }
    }

    // so far so good, go for next event
    ++curr;
  }

  if (includes(final.begin(), final.end(), open.begin(), open.end())) {
    // if all open states are final, it is ok.
    return true;
  }
  else {
    TRACE(3,"Sequence ended but non-final states remain open=["<< set2string(open) <<"]");
    return false;
  }
}

/// Compute possible alignment moves from given marking and log event

set<align_elem> graph::possible_moves(const set<string> &open, const string &event) const {
  TRACE(5,"computing possible moves from ["<< set2string(open) <<"] with event " << event);

  set<align_elem> result;

  // check for possible model or sync moves
  for (auto s : open) { // for each open place
    set<string> succ = get_out_edges(s);
    for (auto t : succ) { // for each possible transition from s
      node tnode = get_node(t);
      // see if all markings to fire that transition are satisfied
      set<string> diff = difference_set(get_in_edges(t), open);
      if (diff.empty()) {
	// all required markings were in 'open', the transition can be fired
	result.insert(align_elem(tnode.id, tnode.name, "[M-REAL]"));  // always add as model move
	if (tnode.name == event) 
	  result.insert(align_elem(tnode.id, tnode.name, "[L/M]"));  // if name matches, add also as sync move
      }
    }
  }

  // log move is always possible
  result.insert(align_elem(DUMMY, event, "[L]"));

  stringstream r; for (auto x : result) r << " " << x.dump(true);  
  TRACE(4,"possible moves computed: [" << r.str() << "]");

  return result;
}

/// Compute resulting marking after applying given possible move to given marking

std::set<std::string> graph::simulate_move(const std::set<std::string> &open, const align_elem &m) const {

  TRACE(5,"Simulating move "<<m.id<<"/"<<m.name<< " from ["<< set2string(open) <<"]");

  set<string> result;
  if (m.type == "[L]") result = open;   // log move, no changes

  else {
    // Sync or model move.
    // New marking is current marking in 'open', removing nodes required to fire the transition, 
    // and adding nodes marked after the firing
    result = fire_transition(open, m.id);
  }
  
  TRACE(5,"Marking after move is ["<< set2string(result) <<"]");
  return result;
}



/// Dump graph in plain text format

string graph::dump() const {
  ostringstream out;
  list<string> nodes = get_nodes_by_id();
  for (auto k : nodes) {
    const node &n = get_node(k);
    if (n.type==node::PLACE)
      out << "PLACE" << "\t" << n.id << "\t" << n.name << "\t"
	  << (is_initial(n.id) ? "INITIAL" : "-") << "\t"  
	  << (is_final(n.id) ? "FINAL" : "-") << endl;
  }
  for (auto k : nodes) {
    const node & n = get_node(k);
    if (n.type==node::TRANSITION)
      out << "TRANSITION" << "\t" << n.id << "\t" << n.name << endl;
  }
  for (auto k : nodes) {
    set<string> edges = get_out_edges(k);
    for (auto d : edges) 
      out << "ARC" << "\t" << k << "\t" << d << endl;
  }
  return out.str();
}
