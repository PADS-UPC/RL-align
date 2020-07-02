
# RL-align

RL-align performs conformance checking aligning log traces with a PM Petri net.
It uses relaxation labeling (RL) constraint satisfaction algorithm to perform the alignment. 

Obtained alignments may be suboptimal, but the computation time is linear with the trace length, offering computation times about two orders of magnitude faster than other state-of-the-art methods.

This software is further described in the following papers:

Version 1.0 (tag ``v1.0`` in gitHub)
   Padró, L., Carmona, J., 2019.
   Approximate computation of alignments of business processes through relaxation labelling,
   in: Business Process Management - 17th International Conference, BPM 2019, pp. 250–267.
   Vienna, Austria, September 1-6, 2019

Version 3 (tag ``v3`` in gitHub)
   [Submitted]
   * Alignments are now suboptimal instead of approximate (i.e. a Fitting alignment is always produced, even if it is not the optimal)
   * Replaced computation of Behavioural Profiles using ``jbpt`` with our own C++ faster code.

## License

RL-align is distributed under an [Affero GPL license](LICENSE).
The code includes the [pugixml parser](https://pugixml.org/) which is distributed under [MIT license](https://opensource.org/licenses/MIT)
The binary for ``punf`` Petri Net Unfolder is also included, dowloaded from [[Victor Khomenko's web page]](http://homepages.cs.ncl.ac.uk/victor.khomenko/tools/tools.html)

## How to install

To run RL-align you'll need a standard linux box with g++5 or later. 
   
### Compile RL-align and subsidiary programs

Go to folder ``src`` and compile RL-align components:
```
   cd src
   make
```

Now you are ready to use the software.

## How to use

### Provided data files

Folder ``data`` contains data files to test the aligner:

  - ``originals`` :  Original pnml models. 
  - ``logs`` : XES trace log files.
  - ``alignments`` : Alignments produced by other State-of-the-art alignment methods
   
You can add you own models and traces, as long as the filenames are consistent, and they are in the right folders.


### Preprocess your models

Some preprocessing is required on your target models before runnig the aligner.
The preprocess consists of computing the unfoldings, the shortest paths between model nodes, and the Behavioral Profiles.

To execute the process run:
```
   bin/preprocess-data.sh 2>err.log
```
(redirecting stderr is recommended since punf is very verbose)


This preprocess will process each model in ``data/originals`` folder and for each of them, it will create in ``data/unfoldings`` the following files:  ``.bp.pnml`` (unfolding), ``.bp`` (behavioral profiles) and ``.path`` (shortest paths files).

This is required only once. After that you can run the aligner as many times as needed.



### Run the aligner

The aligner can be run providing a configuration file and a list of models to be aligned.
```
   bin/execute.sh configfile models
```
On example configuration file is provided: ``config.15.5.-100.-150.-300.cfg``, which produces alignment lower costs on the used tuning dataset.

The model file must be in ``data/unfoldings`` and have extension ``.bp.pnml``. Behavioural profiles and shortest paths should be already precomputed and reside in the same folder. The ``execute.sh`` script expects the trace files to be in ``data/logs`` and have the same name than the model, but with extension ``.xes``.

E.g., to align one model with its corresponding log:
```
   bin/execute.sh config/config.15.5.-100.-150.-300.cfg data/unfoldings/M1.bp.pnml
```
You can also align several models (quotes are important if you do this):
```
   bin/execute.sh config/config.15.5.-100.-150.-300.cfg "data/unfoldings/M1.bp.pnml data/unfoldings/ML3.bp.pnml data/unfoldings/BPIC2017.bp.pnml"
```


This will leave the results in a folder named ``data/results/output.15.5.-100.-150.-300`` (or whatever was the name of the config file)
For each aligned model, a file will be created containing all aligned traces.


### Evaluate results

There are two scripts you can use to evaluate the alignments:
   * The script ``eval-relax.sh`` will evaluate results of relaxation labelling aligner

E.g.
```
   cat data/results/output.15.5.-100.-150.-300/M1.out | bin/eval-relax.sh 
```
You can also compute aggregated results (i.e. to compute microaverage costs and total used CPU time across several models)
```
   cat results/output.15.5.-100.-150.-300/M[0-9]*.out | bin/eval-relax.sh
```

   * The script ``eval-prom.sh`` will evaluate results of aligners in ProM environment, when saved as CSV.

E.g.:
```
   cat data/alignments/ProM-Astar/M1.csv | bin/eval-prom 
```
