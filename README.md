# RL-align

RL-align performs conformance checking aligning log traces with a PM Petri net.
It uses relaxation labeling (RL) constraint satisfaction algorithm to perform the alignment. 

Obtained alignments may be suboptimal or unfitting. However, the computation time is linear with the traec length, offering computation times about two orders of magnitude faster than other state-of-the-art methods.


## License
RL-align is distributed under an [Affero GPL license](LICENSE).
The code includes the [pugixml parser](https://pugixml.org/) which is distributed under [MIT license](https://opensource.org/licenses/MIT)

## How to install

To run RL-align you'll need a standard linux box with g++5 or later. You also will need to have Java SDK installed.

### Install jbpt

Behavioural profiles are computed using jbpt, so you need to install some components.

Just download the following libraries to ``jbpt`` folder:
 * [``jbpt-bp-0.3.1.jar``](https://mvnrepository.com/artifact/org.jbpt/jbpt-bp/0.3.1)
 * [``jbpt-core-0.3.1.jar``](https://mvnrepository.com/artifact/org.jbpt/jbpt-core/0.3.1)
 * [``jbpt-petri-0.3.1.jar``](https://mvnrepository.com/artifact/org.jbpt/jbpt-petri/0.3.1)
   
### Compile RL-align

Go to folder ``src`` and compile RL-align components:
```
   cd src
   make
```

Now you are ready to use the software.


## How to use

### Provided data files

Folder ``data`` contains data files to test the aligner:

  ``originals`` :  Original pnml models
  ``unfoldings`` :  Unfoldings computed using XXXXX
  ``logs`` : XES trace log files 
  ``alignments`` : Reference alignments used to evaluate the results
  ``results`` : empty folder where results will be stored
   
You can add you own models and traces, as long as the filenames are consistent, and they are in the right folders.
   
### Run the aligner

To run the aligner, execute from the main folder:

```
   bin/execute.sh configfile test model
```
E.g., to process one model and one log file:
```
   bin/execute.sh config/config/config.15.5.-100.-20.-300.cfg test data/unfoldigns/M1.bp.pnml
```
You can also launch several models in parallel (quotes are important):
```
   bin/execute.sh config/config/config.15.5.-100.-20.-300.cfg test "data/unfoldigns/M1.bp.pnml data/unfoldigns/M3.bp.pnml data/unfoldigns/M3.bp.pnml"
```


The main folder contains a ``config`` subfolder where different configuration files can be stored.
You can create you configuration by hand, but the scripts ``train.sh`` and ``test.sh`` will do it for you.


