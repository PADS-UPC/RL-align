
#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>

class config {

 public:

    config();
    config(const std::string &fconfig);
    ~config();
  
    /// Relaxation Labelling parameters
    int MAX_ITER=500;
    double SCALE_FACTOR=100.0;
    double EPSILON=0.001;
    
    double DUMMY_INITIAL_WEIGHT = +0.1;
    /// Constraint default compatibilities and other stuff
    double DUMMY_COMPAT = +1.0;
    double EXCLUSIVE_COMPAT = -50.0;
    double CROSS_COMPAT = -10.0;
    double ORDER_COMPAT = +5.0;
    bool ORDER_PROGRESSIVE = false;
    double PARALLEL_COMPAT = +5.0;
    bool PARALLEL_PROGRESSIVE = false;
    double REPEAT_COMPAT = -5.0;
    int MAX_DIST = 3;
    
    // options about unfolding BP
    bool ADD_IFS = false;
    bool ADD_LOOPS = false;
    
};


#endif
