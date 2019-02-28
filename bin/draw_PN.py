##################################################################
##
##    Copyright (C) 2019  Universitat Politecnica de Catalunya
##
##    This library is free software; you can redistribute it and#or
##    modify it under the terms of the GNU Affero General Public
##    License as published by the Free Software Foundation; either
##    version 3 of the License, or (at your option) any later version.
##
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Affero General Public License for more details.
##
##    You should have received a copy of the GNU Affero General Public
##    License along with this library; if not, write to the Free Software
##    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
##    contact: Lluis Padro (padro@cs.upc.es)
##             Computer Science Department
##             Omega-320 - Campus Nord UPC
##             C/ Jordi Girona 31
##             08034 Barcelona.  SPAIN
##
################################################################


# This module, receives an unfolded PNML file and draws it.

# Author: Farbod Taymouri


#Thisi is (added 7 October 2018)
import numpy
import sys
import xml.etree.ElementTree as ET
#import xlsxwriter as XL
import re
import graphviz as gr
import os
import platform


def Reading_Unfold_PNML(net,modelfile):
    tree = ET.parse(modelfile)
    root_initi = tree.getroot()

    #finding the page
    for child in root_initi:

        for c in child:
            print "...", c.tag
            m = re.match('\{.*\}', c.tag)
            # checking whether m is empty or no
            if (m):
                namespace = m.group(0)
            else:
                namespace = ''

            if net=="original" :
               if (c.tag.split("}")[1]=="original_net") :
                  root = c.find(namespace+"net").find(namespace+"page")
                  break

            elif net=="unfolding" :
               if (c.tag.split("}")[1]=="page") :
                  root = c
                  break

        # return place_id, place_name, place_obj
        place_id = []
        place_name = []
        place_obj = []

        transition_id = []
        transition_name = []
        transition_ref=[]
        transition_obj = []

        arcs = []

        temp1 = 0
        temp2 = 0
        temp3 = 0
        init_mark_model = []
        Incident_matrix = []

        # add='C:/Users/University/Desktop/New folder 2/'+ MODEL+'_petri_pnml.xml'
        # add='C:/Users/University/Documents/chert/plg.pnml.xml'
        ##add = model_path
        ##tree = ET.parse(add)
        ##root = tree.getroot()

        # --New Added 30 August 2016-------------------------------------------
        # Some XML files have Namesapce and it is declered at the root like and represented by xmlns like: <pnml xmlns="http://www.pnml.org/version-2009/grammar/pnml">.
        # So in order to acces the tags inside the XML file, the name space must be mentioned!, The general form is like "{name_sapce}tag",
        # For example for reading places tags, the code is like below:
        #   for p in root.iter("{ http://www.pnml.org/version-2009/grammar/pnml }place"):
        #       print p
        # ------
        # First we need to extract the namespace, namely the value of xmlns
        # root.tag='{http://www.pnml.org/version-2009/grammar/pnml}pnml' but we need '{http://www.pnml.org/version-2009/grammar/pnml}'
        # For this issue we use regular expression library (re) of python
        m = re.match('\{.*\}', root.tag)
        # checking whether m is empty or no
        if (m):
            namespace = m.group(0)
        else:
            namespace = ''
        # ------------------------------------------------------------

        # Reading the places ID and its text. place_id is of the form ['n1',..] and  temp1 is of the form {'id': 'n1'}
        # place name is of the form:  ["place_0",...]

        place_ref=[]
        for p in root.iter(namespace + "place"):
            temp1 = p.attrib
            place_id.append(temp1['id'])
            # for p in root.iter("place"):
            temp2 = p.find(namespace + "name").find(namespace + "text").text
            place_name.append(temp2)
            # creating place object
            #temp3 = Places(temp1['id'], temp2)
            place_obj.append(temp3)

            #For unfolded PNML
            if (net=="unfolding") :
                temp22=p.find(namespace + "originalNode").attrib['ref']
            else:
                temp22=""
            place_ref.append(temp22)

        # Reading transitions from PNML file, trnasition_id is of the form ['n318',...], temp1 is of the form {'id': 'n318'}
        # and transition_name is of the form ['A+complete',....]

        transition_Cut_of=[]
        for trans in root.iter(namespace + "transition"):
            temp1 = trans.attrib
            transition_id.append(temp1['id'])
            # for trans in root.iter("transition"):
            temp2 = trans.find(namespace + "name").find(namespace + "text").text
            if (net=="unfolding") :
                temp22= trans.find(namespace + "originalNode").find(namespace + "text").text
            else:
                temp22= temp2

            #Adding cut-off attribute
            try:
                tempCut = trans.find(namespace + "cut-off").find(namespace + "correspondent").find(namespace + "ref").text
            except AttributeError:
                #Cut-off is not available
                tempCut=0



            # This part was added to work with realistc data set, bpi2012
            # Here we changed the "_" in the name of transition with "*"
            # -------
            # some real transitions of the model is like A\\n, B\\n, we must remove \\n
            # the ones come from PLG model generator
            # temp2=temp2.strip("\\n")

            '''if (" " in temp2):
                temp2 = temp2.replace(" ", "*")

            if ("_" in temp2):
                temp2 = temp2.replace("_", "*")'''
            # -----------
            transition_Cut_of.append(tempCut)
            transition_ref.append(temp22)
            transition_name.append(temp2)
            #temp3 = Transitions(temp1['id'], temp2)
            transition_obj.append(temp3)

        # Reading arcs for PNML, the lis arcs is of the form arcs=[{'source': 'n78', 'id': 'arc635', 'target': 'n447'},....]
        for ar in root.iter(namespace + "arc"):
            arcs.append(ar.attrib)

        # Calculating initial marking from PNML
        # init_mark is of the form [None, None, None, None, None, None, None, <Element 'initialMarking' at 0xa27ad0c>, None,.....]

        # init_mark_model is of the form=[None, None, None, None, None, None, None, <Element 'initialMarking' at 0x15a128d0> ,....]
        ##for place in root.iter(namespace + "place"):
            ##init_mark_model.append(place.find(namespace + "initialMarking"))

        # init_mark_model_list = []
        # for i in range(len(init_mark_model)):
        #     if (init_mark_model[i] != None):
        #         init_mark_model_list.append(1)
        #     else:
        #         init_mark_model_list.append(0)
        # initial_place_marking = place_name[init_mark_model_list.index(1)]

        # Calculating incident matrix using Matrix method
        Incident_matrix = Matrix(transition_id, place_id, arcs)

      

        print transition_Cut_of
        Drawing_Model(transition_id, transition_obj,
                      transition_ref, transition_Cut_of, place_id, place_name, place_obj, place_ref, Incident_matrix, os.getcwd())
        #return transition_id, transitaion_name_modified, transition_obj, place_id, place_name, place_obj, Incident_matrix
        # return place_id, place_name, place_obj


##############################################################################
#This is a temporary method related to the above method (added 7 october 2018)
def  Drawing_Model(transition_id, transition_obj,
                   transition_ref, transition_Cut_of, place_id, place_name, place_obj, place_ref, Incident_matrix,destination):
    # path=V.destination_path


    places=place_id
    transitions=transition_id
    incident_matrix=Incident_matrix

    # places = V.p_name[:]
    # transitions = V.transition[:]
    # incident_matrix = V.Inc_matrix[:]

    model_move = []
    log_move = []



    # Now attempting to create a graph

    dot = gr.Digraph()

    for i in range(len(places)):
        dot.node(str(places[i] + "," + place_ref[i]), shape="circle")

    # Here during the initializing the transition nodes of the graph, if we encountered the transition which is appeared in the net_moves,
    # we highlight it and also assign it a moves number.
    # Similarly transitions which are asynch moves or Skipped, will receive another colors
    for i in range(len(transitions)):

             dot.node(str(transitions[i]+","+transition_ref[i]), shape="rect", style='filled', color = "red" if transition_Cut_of[i]!=0 else 'lightblue2' )


        #else:
            #dot.node(transitions[i], shape="rect")

    for i in range(len(incident_matrix)):
        for j in range(len(incident_matrix[0])):
            if (incident_matrix[i][j] == 1):

                dot.edge(str(transitions[j]+","+transition_ref[j]), str(places[i] + "," + place_ref[i]))


            elif (incident_matrix[i][j] == -1):

                dot.edge(str(places[i] + "," + place_ref[i]), str(transitions[j]+","+transition_ref[j]))

        #Graphviz must be installed in order to save output in pdf format
    if platform.system()== "Windows":
    	os.environ["PATH"] += os.pathsep + 'C:/Program Files (x86)/Graphviz2.38/bin/'
    elif platform.system()== "Linux":
    	if('graphviz' not in sys.modules):
    		os.system('sudo apt-get install graphviz')


    print "the net move is figured"
    f = open(destination + "/Graph_net_moves_color.dot", "w")
    dot.render("plot.png")
    f.write(dot.source)

####################################################################################

# This is an internal function
def Matrix(transitions, places, arcs):
    # trnasition_id is of the form ['n318',...],
    # place_id is of the form ['n1',..]
    # arcs is of the form arcs=[{'source': 'n78', 'id': 'arc635', 'target': 'n447'},....]
    incident_matrix = numpy.zeros((len(places), len(transitions)), dtype=numpy.int)

    for i in range(len(arcs)):
        s = arcs[i]['source']
        t = arcs[i]['target']

        if ((s in transitions) and (t in places)):
            pl = places.index(t)
            tr = transitions.index(s)

            incident_matrix[pl][tr] = 1

        elif ((t in transitions) and (s in places)):
            pl = places.index(s)
            tr = transitions.index(t)

            incident_matrix[pl][tr] = -1

    return incident_matrix

# #############################################################################
# #This is a temporary method related to the above method (added 7 october 2018)
# def  Drawing_Model(transition_id, transition_obj,
#                    transition_ref,place_id, place_name, place_obj, Incident_matrix,destination):
#     # path=V.destination_path


#     places=place_id
#     transitions=transition_id
#     incident_matrix=Incident_matrix

#     # places = V.p_name[:]
#     # transitions = V.transition[:]
#     # incident_matrix = V.Inc_matrix[:]

#     model_move = []
#     log_move = []



#     # Now attempting to create a graph

#     dot = gr.Digraph()
#     for i in range(len(places)):
#         dot.node(places[i], shape="circle")

#     # Here during the initializing the transition nodes of the graph, if we encountered the transition which is appeared in the net_moves,
#     # we highlight it and also assign it a moves number.
#     # Similarly transitions which are asynch moves or Skipped, will receive another colors
#     for i in range(len(transitions)):
#             dot.node(str(transitions[i]+","+transition_ref[i]), shape="rect")

#         #else:
#             #dot.node(transitions[i], shape="rect")

#     for i in range(len(incident_matrix)):
#         for j in range(len(incident_matrix[0])):
#             if (incident_matrix[i][j] == 1):

#                 dot.edge(str(transitions[j]+","+transition_ref[j]), places[i])


#             elif (incident_matrix[i][j] == -1):

#                 dot.edge(places[i], str(transitions[j]+","+transition_ref[j]))


#     #Graphviz must be installed in order to save output in pdf format
#     if platform.system()== "Windows":
#     	os.environ["PATH"] += os.pathsep + 'C:/Program Files (x86)/Graphviz2.38/bin/'
#     elif platform.system()== "Linux":
#     	if('graphviz' not in sys.modules):
#     		os.system('sudo apt-get install graphviz')


#     print "The model is figured"
#     f = open(destination + "/Graph_net_moves_color.dot", "w")
#     dot.render("plot.png")
#     f.write(dot.source)









if __name__ == "__main__":

    print "The input model is:", sys.argv[1], sys.argv[2]
    Reading_Unfold_PNML(sys.argv[1], sys.argv[2])
