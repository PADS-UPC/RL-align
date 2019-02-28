
#ifndef __GRAPH_H
#define __GRAPH_H

#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>

#include "pugixml.hpp"
#include "alignment.h"

class node {
  public:
    typedef enum {PLACE, TRANSITION} NodeType;
    NodeType type;
    std::string id;
    std::string name;
    bool initial_marking;

    node();
    node(NodeType t, const std::string &i,
         const std::string &n,
	 bool init=false);
    ~node();
};


class graph {

   public:
     typedef enum {ORIGINAL, UNFOLDING} NetVariant;

   private:
     // nodes by id 
     std::map<std::string,node> nodes_by_id;
     // nodes by name
     std::multimap<std::string,std::string> nodes_by_name;
     // target node id for edges getting out from a node, by id
     std::multimap<std::string,std::string> out_edges;
     // source node id for edges getting in a node, by id
     std::multimap<std::string,std::string> in_edges;
     // id for initial node
     std::set<std::string> initial_nodes;
     // id for final node
     std::set<std::string> final_nodes;
     // distances between nodes. map keys are "id1 id2"
     std::map<std::string,double> distances;
     // shortest paths between nodes. Distance==-1 means no path.
     std::map<std::string,std::list<std::string>> paths;
     
     // auxiliary: check for already existing nodes
     std::map<std::string,node>::const_iterator check_node(const std::string &id) const;
     // auxiliary: get edges in or out from a node
     std::set<std::string> get_edges(const std::string &id,
                                     const std::multimap<std::string,std::string> &edges) const;
     // auxiliary: load nodes of given type from XML file
     void load_nodes(pugi::xml_node page, const std::string &type, NetVariant which);     

     /// utility: remove a pair (key,val) from given multimap
     static void remove_from_multimap(std::multimap<std::string,std::string> &mmap, const std::string &key, const std::string &val);
     /// utility: find out if there is a loop involving given node, or any accessible from it.
     bool has_loops(const std::string &node, std::set<std::string> &seen) const;

     std::string find_matching_join(const std::string &id, int depth, std::set<std::string> &seen) const;
     
   public:
     // dummy label
     static const std::string DUMMY;

     graph();
     graph(const std::string &fname, NetVariant which, bool addIFS=false, bool addLOOPS=false);
     ~graph();

     void add_node(const node &n);
     void add_edge(const std::string &src, const std::string &targ);
     void remove_node(const std::string &id);
     void remove_edge(const std::string &src, const std::string &targ);

     void compute_distances();
     void load_distances(const std::string &fname);
     void save_distances(std::ostream &sdist) const;

     void load_paths(const std::string &fname);
     
     std::set<std::string> get_initial_nodes() const;
     std::set<std::string> get_final_nodes() const;
     bool is_initial(const std::string &id) const;
     bool is_final(const std::string &id) const;
     const node& get_node(const std::string &id) const;
     bool is_leaf(const std::string &id) const;
     int get_num_nodes() const;
     std::list<std::string> get_nodes_by_id() const;
     std::list<std::string> get_nodes_by_name(const std::string &id) const;
     std::set<std::string> get_out_edges(const std::string &id) const;
     std::set<std::string> get_in_edges(const std::string &id) const;
     bool is_parallel_join(const std::string &id) const;
     bool is_parallel_split(const std::string &id) const;
     std::string find_matching_split(const std::string &id) const;
     std::string find_matching_join(const std::string &id) const;
     bool connected(const std::string &src, const std::string &targ) const;
     bool accessible(const std::string &src, const std::string &targ) const;
     bool path_exists(const std::string &src, const std::string &targ) const;
     double distance(const std::string &id1, const std::string &id2) const;
     std::list<std::string> path(const std::string &id1, const std::string &id2) const;
     bool is_acyclic() const;
     bool is_fitting(std::set<std::string> &open,
                     const std::set<std::string> &final,
                     alignment::iterator from, alignment::iterator to,
                     alignment::iterator &curr,
                     bool skip_unexpected=false) const;

     std::list<align_elem> possible_moves(const std::set<std::string> &open, const std::string &event) const;
     std::set<std::string> simulate_move(const std::set<std::string> &open, const align_elem &m) const;

     std::string dump() const;
};

#endif
