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
#include <ctime>
#include <climits>
#include <algorithm>
#include <vector>

#include "util.h"
#include "graph.h"
#include "relax.h"
#include "bp.h"
#include "config.h"
#include "alignment.h"
#include "traces.h"
#define MOD_TRACENAME "ALIGN"
#define MOD_TRACECODE MAIN_TRACE

using namespace std;

// configuration
config *cfg;


///////////////////////////////////////////////////////
/// add labels (possible alignments) to variables (trace events)
/// in a RL constraint satisfaction problem

void add_variable_labels(problem & prob,
                         const vector<string> &trace,
                         const graph& g) {

  // add possible alignments for each event
  for (size_t nv=0; nv<trace.size(); ++nv) {
    TRACE(2, "Adding variable " << nv << " " << trace[nv]);
    prob.set_var_name(nv, trace[nv]);
    // get candidates to be aligned
    list<string> labels = g.get_nodes_by_name(trace[nv]);
    // add them as labels for variable nv
    int nl=0;
    for (auto id : labels) {
      TRACE(2, "     label " << nl << " " << id);
      prob.add_label(nv, (1.0-cfg->DUMMY_INITIAL_WEIGHT)/labels.size(), id);
      ++nl;
    }
    // add an extra dummy label
    prob.add_label(nv, cfg->DUMMY_INITIAL_WEIGHT, graph::DUMMY);
  }
}

///////////////////////////////////////////////////////
/// compute difference between events ev1,ev2 position
//  in the trace to the distance in tho model between candidate
//  tasks lb1,lb2.

double distance_balance(int ev1, int lb1, int ev2, int lb2, const graph& g, const problem& prob, bool progressive) {

  if (not progressive) return 1.0;

  string lname1 = prob.get_label_name(ev1,lb1);
  string lname2 = prob.get_label_name(ev2,lb2);                       

  double dg = g.distance(lname1, lname2)+1;

  double dt = abs(ev1-ev2);  // distance in the trace

  TRACE(4, "  distance_balance "<<lname1<<" "<<lname2<<" dg="<<dg<<" dt="<<dt);

  return abs(dt-dg)+1;
}

///////////////////////////////////////////////////////
/// add constraints (BP restrictions) between
/// labels (possible alignments)

void add_constraints(problem & prob,
                     const vector<string> &trace,
                     const behavioral_profile &bp,
                     const behavioral_profile &bptf,
                     const graph &g) {

  // longest ngram to consider (all the length if MAX_DIST==0)
  int md = (cfg->MAX_DIST!=0 ? cfg->MAX_DIST : 2*g.get_num_nodes()); 

  int M = trace.size();
  for (int ev1=0; ev1<M; ++ev1) {
    for (int ev2=ev1+1; ev2<=std::min(M-1, ev1+md); ++ev2) {
      for (int lb1=0; lb1<prob.get_num_labels(ev1); ++lb1) {
        for (int lb2=0; lb2<prob.get_num_labels(ev2); ++lb2) {

          string t1 = prob.get_label_name(ev1,lb1);
          string t2 = prob.get_label_name(ev2,lb2);
          
          if (t1 == graph::DUMMY or t2 == graph::DUMMY)
            continue;
          
          else if (cfg->REPEAT_COMPAT!=0 and
                   prob.get_label_name(ev1,lb1) == prob.get_label_name(ev2,lb2)) {
            TRACE(4, "Repeat compatibility constraint");
            prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->REPEAT_COMPAT);            
            prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->REPEAT_COMPAT);
          }
          
          else {
            switch (bp.get_relation(t1,t2)) {
              case behavioral_profile::NO_RELATION : {
                 ERROR_CRASH("Invalid or missing BP relation for pair "
                             << prob.get_label_name(ev1,lb1) << " "
                             << prob.get_label_name(ev2,lb2) );                 
              }
                
              case behavioral_profile::PRECEDES :
                if (cfg->ORDER_COMPAT!=0) {
                  double diff;
                  diff = distance_balance(ev1, lb1, ev2, lb2, g, prob, cfg->ORDER_PROGRESSIVE);
                  
                  TRACE(4, "Order compatibility constraint. diff="<<diff);
                  prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->ORDER_COMPAT/diff);
                  prob.add_constraint(ev2, lb2, {{make_pair(ev1,lb1)}}, cfg->ORDER_COMPAT/diff);
                }
                break;
              
              case behavioral_profile::FOLLOWS :
                if (cfg->CROSS_COMPAT!=0) {
                  TRACE(4, "Cross compatibility constraint");
                  prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->CROSS_COMPAT);
                  prob.add_constraint(ev2, lb2, {{make_pair(ev1,lb1)}}, cfg->CROSS_COMPAT);
                }
                break;
              
              case behavioral_profile::EXCLUSIVE :
                if (cfg->EXCLUSIVE_COMPAT!=0) {
                  TRACE(4, "Exclusive compatibility constraint");
                  prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->EXCLUSIVE_COMPAT);
                  prob.add_constraint(ev2, lb2, {{make_pair(ev1,lb1)}}, cfg->EXCLUSIVE_COMPAT);
                }
                break;
              
              case behavioral_profile::INTERLEAVED :
                if (cfg->PARALLEL_COMPAT!=0) {
                  TRACE(4, "Parallel compatibility constraint");
                  double diff;
                  // "real" paralels get no penalty for long paths
                  if (bptf.get_relation(t1,t2)==behavioral_profile::INTERLEAVED) diff = 1;
                  else diff = distance_balance(ev1, lb1, ev2, lb2, g, prob, cfg->PARALLEL_PROGRESSIVE);
                  prob.add_constraint(ev1, lb1, {{make_pair(ev2,lb2)}}, cfg->PARALLEL_COMPAT/diff);

                  if (bptf.get_relation(t2,t1)==behavioral_profile::INTERLEAVED) diff = 1;
                  else diff = distance_balance(ev2, lb2, ev1, lb1, g, prob, cfg->PARALLEL_PROGRESSIVE);
                  prob.add_constraint(ev2, lb2, {{make_pair(ev1,lb1)}}, cfg->PARALLEL_COMPAT/diff);
                }
                break;
            }
          }
        }
      }
    }
  }

  if (cfg->DUMMY_COMPAT!=0) {
    // add constraints favoring dummy label ([L])
    // Given events A X B, if aligning X would create a path between A and B longer than
    // the A-B path if X is ommited, penalize the alignment of X
    for (int ev=1; ev<M-1; ++ev) {      
      int evL = ev-1;
      int evR = ev+1;
      for (int lb=0; lb<prob.get_num_labels(ev)-1; ++lb) {  // all labels except DUMMY
        string te = prob.get_label_name(ev,lb);
        for (int lbL=0; lbL<prob.get_num_labels(evL)-1; ++lbL) { // all labels except DUMMY
          for (int lbR=0; lbR<prob.get_num_labels(evR)-1; ++lbR) { // all labels except DUMMY
            string tL = prob.get_label_name(evL,lbL);
            string tR = prob.get_label_name(evR,lbR);

            double dLR = -1;
            if (bptf.get_relation(tL,tR)==behavioral_profile::INTERLEAVED) dLR = 0;
            else if (g.path_exists(tL,tR)) dLR = g.path(tL,tR).size()/2.0;
            //else dLR = g.get_num_nodes()*2;

            double dLe = -1;
            if (bptf.get_relation(tL,te)==behavioral_profile::INTERLEAVED) dLe = 0;
            else if (g.path_exists(tL,te)) dLe = g.path(tL,te).size()/2.0;
            //else dLe = g.get_num_nodes()*2;

            double deR = -1;
            if (bptf.get_relation(te,tR)==behavioral_profile::INTERLEAVED) deR = 0;
            else if (g.path_exists(te,tR)) deR = g.path(te,tR).size()/2.0;
            //else deR = g.get_num_nodes()*2;
            
            TRACE(5, "checking Dummy compatibility constraint "<<tL<<"-["<<te<<"]-"<<tR<<" "<<dLR<<" "<<dLe<<" "<<deR );
            if (dLR>=0 and dLe>=0 and deR>=0 and dLR < dLe+deR-1) {
              TRACE(4, "Dummy compatibility constraint");
              prob.add_constraint(ev, lb, {{make_pair(evL,lbL)},{make_pair(evR,lbR)}}, cfg->DUMMY_COMPAT*(dLe+deR-1-dLR) );
            }
          }
        }
      }
    }
  }

}

///////////////////////////////////////////////////////
/// create a constraint satisfaction problem,
/// add fill it using event, alignment, and BP information

problem create_labeling_problem(const vector<string> &trace,
                                          const graph& g,
                                          const behavioral_profile &bp,
                                          const behavioral_profile &bptf) {

  // each trace event is a variable in our alignment problem
  problem prob(trace.size());
  // add possible alignments for each event (i.e. possible labels for each variable)
  TRACE(1, "Adding variables ");
  add_variable_labels(prob,trace,g);
  // create constraints according to BP
  TRACE(1, "Adding constraints ");
  add_constraints(prob,trace,bp,bptf,g); 
      
  return prob;
}



///////////////////////////////////////////////////////
/// load traces from a .xes file

list<pair<string,vector<string>>> load_traces(const string &fname, const graph &g) {
    // open input file
    pugi::xml_document xmldoc;
    xmldoc.load_file(fname.c_str(), pugi::parse_default|pugi::parse_ws_pcdata);
    TRACE(1, "loaded trace file");

    list<pair<string,vector<string>>> result;

    // get each "trace" node under "log"
    pugi::xml_node trace = xmldoc.child("log").child("trace");
    
    map<string,int> warned;
    while (trace) {
      string trace_id = trace.find_child_by_attribute("string","key","concept:name").attribute("value").value();
      TRACE(7, "found new trace id="<<trace_id);
      // get each event of the trace
      vector<string> evs;
      pugi::xml_node event = trace.child("event");
      while (event) {
        pugi::xml_node name = event.find_child_by_attribute("string","key","concept:name");
        string evname = name.attribute("value").value();
        evs.push_back(evname);
        TRACE(7, "   read event "<<evname);
        if (g.get_nodes_by_name(evname).empty()) {
          if (warned.find(evname)==warned.end()) warned.insert(make_pair(evname,1));          
          else warned[evname] += 1;
        }
        event = event.next_sibling("event");
      }
      result.push_back(make_pair(trace_id,evs));

      trace = trace.next_sibling("trace");
    }

    for (auto w : warned) {
      WARNING("WARNING: Event name '"<<w.first<<"' occurred "<<w.second<<" times in the log, but no matching model task was found.");
    }
    
    return result;   
}


///////////////////////////////////////////////////////
/// create an alignment from a solved RL problem

alignment RL_to_alignment(const graph& g, const pair<string,vector<string>> &trace, const problem& prob) {
  
  alignment seq;
  for (auto n : g.get_initial_nodes())
    seq.push_back(align_elem(n,"^","[ANCHOR]")); // initial place, to anchor the sequence
  
  for (size_t nv=0; nv<trace.second.size(); ++nv) {
    int best = prob.best_label(nv).front();
    string var1 = prob.get_var_name(nv);
    string lab1 = prob.get_label_name(nv,best);
    
    if (lab1 == graph::DUMMY) seq.push_back(align_elem(lab1,var1,"[L]"));
    else seq.push_back(align_elem(lab1,var1,"[L/M]"));
  }
  
  for (auto n : g.get_final_nodes())
    seq.push_back(align_elem(n,"^","[ANCHOR]")); // final place, to anchor the sequence
  
  return seq;
}

  
///////////////////////////////////////////////////////
/// add missing model moves to an alignment

void add_model_moves(alignment &seq, const graph &g, const behavioral_profile &bptf) {
  alignment::iterator p = seq.begin();
  alignment::iterator pprev = p; 
  ++p;  // start at second element (first is the anchor, initial place)
  while (p!=seq.end()) {
    
    if (p->type != "[L]") {   // skip deletions
      
      if (g.path_exists(pprev->id,p->id) and bptf.get_relation(pprev->id,p->id)!=behavioral_profile::INTERLEAVED) {
        list<string> mreal = g.path(pprev->id,p->id);
        for (auto m : mreal) {
          if (g.get_node(m).type == node::TRANSITION)
            seq.insert(p, align_elem(m, g.get_node(m).name, "[M-REAL]"));
        }
      }
      
      pprev = p; // 'pprev' remembers is the last non-deletion element seen
    }
    
    ++p;
  }
  
}


/// ===========================
/// ========= MAIN ============
/// ===========================

int main(int argc, char *argv[]) {

  if (argc<4) {
    ERROR_CRASH("Usage " << argv[0] << " model-prefix traces config [tracingoptions]\n         tracingoptions format is level:hexmask. eg. 4:0x103\n         e.g.: "<<argv[0] << "modelsdir/M1 logsdir/M1.xes configdir/cfile.cfg");
  }

  // load parameters
  string basename(argv[1]);
  string ftrace(argv[2]);
  string fconfig(argv[3]);
  cfg = new config(fconfig);

  traces::set_tracing(argc>4 ? string(argv[4]) : "");
  
  string modelfile = basename+".bp.pnml";
  TRACE(1, "Loading graph..."<<modelfile); // load XML model file
  graph g(modelfile, graph::UNFOLDING, cfg->ADD_IFS, cfg->ADD_LOOPS); 
  g.add_node(node(node::TRANSITION, graph::DUMMY, graph::DUMMY));  // add "dummy" node

  string fpaths = basename+".tt.path";
  TRACE(1, "Loading paths..."<<fpaths);
  g.load_paths(fpaths);

  // bptf is BP without loops (used to detect "real" parallels)
  string fbp = basename+".tf.bp";
  TRACE(1, "Loading BPs..."<<fbp);
  behavioral_profile bptf(fbp);
  TRACE(7, "BP loaded is: " << bptf.dump() );

  // bp is BP defined by config file (tf: w/o loops, tt: w/ loops)
  fbp = basename+".tt.bp";
  TRACE(1, "Loading BPs..."<<fbp);
  behavioral_profile bp(fbp);
  TRACE(7, "BP loaded is: " << bp.dump());

  TRACE(1, "Loading trace file " << ftrace);
  list<pair<string,vector<string>>> log = load_traces(ftrace,g);  // load traces
  TRACE(1, "Loaded " << log.size() << " traces...");

  /// Create a RL solver for the constraint satisfaction problems
  relax solver(cfg->MAX_ITER, cfg->SCALE_FACTOR, cfg->EPSILON);

  for (auto trace : log) {
    // try to align trace and graph.
    TRACE(1, "-----------------------------------------------------");
    TRACE(0, "ALIGNING TRACE " << trace.first );

    clock_t t0 = clock();  // initial time

    // create constraint satisfaction problem 
    problem prob = create_labeling_problem(trace.second, g, bp, bptf);

    // solve constraint satisfaction problem using RL
    TRACE(1, "  solving RL problem");
    solver.solve(prob);

    TRACE(1, "  solved. Adding model moves");
    
    // extract solution and create a (partially) aligned sequence
    alignment seq = RL_to_alignment(g, trace, prob);
    TRACE(3, "initial alignment: "<< seq.dump());
    TRACE(3, "initial alignment: "<< seq.dump(true));

    /*  --------------- BEGIN OF NEW COMPLETION PROPOSAL -------------*/
    set<string> open = g.get_initial_nodes();
    auto p = seq.begin();
    ++p; // skip anchor
    while (p != seq.end()) {
      if (p->type=="[L]") {
        ++p;
        continue;
      }

      // p->type is [L/M]. Find a path to p current PN state (maybe empty if p can already be fired)
      list<string> mreal;
      if (g.find_path(open, p->id, mreal)) {        
        // there is a path that can fill the gap:  Fill the gap with the shortest path
        for (auto m : mreal) {
          if (g.get_node(m).type == node::TRANSITION) {
            seq.insert(p, align_elem(m, g.get_node(m).name, "[M-REAL]"));
            open = g.fire_transition(open, m);
          }
        }
        // we are good up to p, move to next event
        open = g.fire_transition(open, p->id);
        ++p;
        continue;
      }

      // p->type is [L/M], and there is no possible set of model moves that will fix this.
      // Try removing p, and cross fingers.
      p->type = "[L]";
    }

    
    /*  --------------- END OF NEW COMPLETION PROPOSAL -------------*/

    /*  --------------- BEGIN OF OLD COMPLETION PROPOSAL -------------    
    // complete sequence with missing model moves
    add_model_moves(seq, g, bptf);    
    TRACE(1, "Completed alignment ");
    TRACE(3, "Completed alignment: "<< seq.dump());
    TRACE(3, "Completed alignment: "<< seq.dump(true));

    p = seq.begin(); ++p;
    while (p!=seq.end()) {

      // skip deletions, and look for a parallel split
      if (p->type=="[L]" or not g.is_parallel_split(p->id)) {
        ++p;
        continue;
      }

      TRACE(3, "found split "<<p->id);
      
      // it is a parallel split, find its closing join
      string match = g.find_matching_join(p->id);
      TRACE(3, "parallel join "<<match<<" found to match split "<<p->id);
      
      // locate closing join in the trace, ahead of p
      alignment::iterator q(p);  
      while (q!=seq.end() and (q->type=="[L]" or q->id!=match) ) ++q;
      if (q==seq.end()) {
        TRACE(1, "No matching parallel join for "<< p->id <<" was found in the trace. Likely non-fitting trace");
        break;
      }
      
      set<string> open = g.get_in_edges(p->id);
      set<string> final = g.get_out_edges(q->id);
      alignment::iterator pos;
      while (not g.is_fitting(open, final, p, q, pos, false)) {
        TRACE(3, "parallel section not fitting! "<<p->id<<" "<<match);
        
        // see which states remained opened and shouldn't.
        set<string> remaining = difference_set(open,final);
        
        TRACE(3, " nonfinal remaining opened =["<<set2string(remaining)<<"]   pos="<<pos->id);

        // shortest non-empty path from opened nonfinal to pos.
        string missing;
        size_t min=9999999;
        for (auto r : remaining) {
          if (g.path_exists(r,pos->id)) {
            list<string> p = g.path(r,pos->id);
            if (p.size()>0 and p.size()<min) {
              min = p.size();
              missing = p.front();
            }
          }
        }
       
        if (missing.empty()) {
          TRACE(1, "No path found to complete unfitting parallel. Likely non-fitting trace");
          break;
        }

        TRACE(3, "   inserting ["<<missing<<"] before pos="<<pos->id);
        seq.insert(pos, align_elem(missing, g.get_node(missing).name, "[M-REAL]"));

        // restore original list, in case we need to loop again
        open = g.get_in_edges(p->id);
      }

      p = q; // we made it fitting from p to q, skip ahead.
    }
      --------------- END OF OLD COMPLETION PROPOSAL -------------*/
          
    seq.pop_front(); // remove initial node anchor.
    seq.pop_back(); // remove final node anchor.

    TRACE(1, "Final alignment ");
    TRACE(3, "Final alignment: "<< seq.dump());
    TRACE(3, "Final alignment: "<< seq.dump(true));

    seq.purge();
    TRACE(1, "Purged alignment ");
    TRACE(3, "Purged alignment: "<< seq.dump());
    TRACE(3, "Purged alignment: "<< seq.dump(true));

    cout << trace.first << " " << seq.dump();
        
    while (not seq.empty() and seq.front().type=="[L]") // skip initial [L] elements, if any
      seq.pop_front();

    set<string> initial = g.get_initial_nodes();
    set<string> final = g.get_final_nodes();

    alignment::iterator pos;
    alignment::iterator last = seq.end();
    --last;
    if (g.is_fitting(initial, final, seq.begin(), last, pos)) cout << " FITTING";
    else cout << " NOT-FITTING";

    clock_t t1 = clock();  // final time
    cout << " " << double(t1-t0)/double(CLOCKS_PER_SEC);

    cout << endl;
  }
}


