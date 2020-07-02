
#ifndef __GRAPH_H
#define __GRAPH_H

#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>

#include "pugixml.hpp"
#include "alignment.h"


// auxiliar class for BFS path searchs
class search_state {
  public:
     std::set<std::string> open_places;
     std::list<std::string> path;
     int distance;
     search_state();
     search_state(const std::set<std::string> &op, const std::list<std::string> &pth, int dist);
     ~search_state();
     bool operator<(const search_state &p) const;
};


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
     // matching parallel joins for each parallel split
     std::map<std::string,std::string> parallels;
     
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
    
   public:
     // dummy label
     static const std::string DUMMY;
     static size_t BFS_LIMIT;
     static size_t NUM_SAMPLE_PATHS;

     graph();
     graph(const std::string &fname, NetVariant which, bool addIFS=false, bool addLOOPS=false);
     ~graph();

     void add_node(const node &n);
     void add_edge(const std::string &src, const std::string &targ);
     void remove_node(const std::string &id);
     void remove_edge(const std::string &src, const std::string &targ);
     void replace_node(const std::string &oldnode, const std::string &newnode);

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
     bool is_exclusive_join(const std::string &id) const;
     bool is_parallel_split(const std::string &id) const;
     bool is_exclusive_split(const std::string &id) const;
     void set_parallel(const std::string &split, const std::string &join);
     std::string get_parallel_join(const std::string &split) const;
     const std::map<std::string,std::string> & get_parallels() const;
     bool connected(const std::string &src, const std::string &targ) const;
     bool accessible(const std::string &src, const std::string &targ) const;
     bool path_exists(const std::string &src, const std::string &targ) const;
     void get_branch_nodes(const std::string &src, const std::string &targ, std::list<std::string> &branch) const;

     double distance(const std::string &id1, const std::string &id2) const;
     int shortest_distance(const std::set<std::string> &open, const std::string &target) const;
     int average_distance(const std::set<std::string> &open, const std::string &target) const;

     std::list<std::string> path(const std::string &id1, const std::string &id2) const;
     bool is_acyclic() const;
     bool is_fitting(std::set<std::string> &open,
                     const std::set<std::string> &final,
                     alignment::iterator from, alignment::iterator to,
                     alignment::iterator &curr,
                     bool skip_unexpected=false) const;

     std::set<align_elem> possible_moves(const std::set<std::string> &open, const std::string &event) const;
     std::set<std::string> simulate_move(const std::set<std::string> &open, const align_elem &m) const;
 
     std::set<std::string> possible_transitions(const std::set<std::string> &open, std::string target="") const;
     bool is_final(const std::set<std::string> &open) const;     /// see if a PN configuration is final
     std::set<std::string> fire_transition(const std::set<std::string> &open, const std::string &t) const;
     int estimated_cost(const search_state & st, const std::string &target) const;
     search_state next_search_state(const search_state & current, const std::string & t) const;
     bool find_path(const std::set<std::string> &open, const std::string &target, std::list<std::string> &path) const;
     bool random_path(const std::string &n, const std::string &s, std::list<std::string> &path) const;
     std::list<std::string> find_path_by_sampling(const std::string &n, const std::string &target) const;
     std::string dump() const;

};

#endif
