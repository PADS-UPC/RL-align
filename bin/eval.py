#! /usr/bin/python3

import sys
import os

GOLD_FITTING=0
GOLD_ALL=1
noGOLD_FITTING=2
noGOLD_ALL=3

prefix=[0,0,0,0]
prefix[GOLD_FITTING]="gFit"
prefix[GOLD_ALL]="gAll"
prefix[noGOLD_FITTING]="ngFit"
prefix[noGOLD_ALL]="ngAll"

# -------------------------------
# print stats

def stats(name,Tok,Ttot,Etot,EokLM,EokL,Epred,Eexp,Icost,SGcost,SRcost,TimeTot) :
   print (name)
   print ("With reference solution")
   F = 100.0*Ttot[GOLD_FITTING]/Ttot[GOLD_ALL]
   print ("   Fitting: {:.2f}% ({}/{})".format(F,Ttot[GOLD_FITTING],Ttot[GOLD_ALL]))   
   for k in [GOLD_FITTING,GOLD_ALL] :
      if Ttot[k]==0 : continue
      N = prefix[k]
      A = 100.0*(EokLM[k]+EokL[k])/Etot[k]
      print ("   {}_Acc: {:.2f}% (({}+{})/{})".format(N,A,EokLM[k],EokL[k],Etot[k]))
      P = 100.0*EokLM[k]/Epred[k]
      print ("   {}_P: {:.2f}% ({}/{})".format(N,P,EokLM[k],Epred[k]))
      R = 100.0*EokLM[k]/Eexp[k]
      print ("   {}_R: {:.2f}% ({}/{})".format(N,R,EokLM[k],Eexp[k]))
      print ("   {}_F1: {:.2f}%".format(N,2*P*R/(P+R) if P+R>0 else 0))
      W = 100.0*Tok[k]/Ttot[k]
      print ("   {}_Identical: {:.2f}% ({}/{})".format(N,W,Tok[k],Ttot[k]))
      C = 100.0*Icost[k]/Ttot[k]
      print ("   {}_SameCost: {:.2f}% ({}/{})".format(N,C,Icost[k],Ttot[k]))
      DR = 1.0*SRcost[k]/Ttot[k]
      print ("   {}_AvgCostSol: {:.2f} ({}/{})".format(N,DR,SRcost[k],Ttot[k]))
      DG = 1.0*SGcost[k]/Ttot[k]
      print ("   {}_AvgCostRef: {:.2f} ({}/{})".format(N,DG,SGcost[k],Ttot[k]))
      DF = 1.0*(SRcost[k]-SGcost[k])/Ttot[k]
      print ("   {}_AvgCostDiff: {:.2f} ({}/{})".format(N,DF,SRcost[k]-SGcost[k],Ttot[k]))
      TT = 1.0*TimeTot[k]/Ttot[k]
      print ("   {}_AvgTraceTime: {:.6} ({:.1f}/{})".format(N,TT,1.0*TimeTot[k],Ttot[k]))
      print ("   {}_TotalTime: {:.1f} ({} traces)".format(N,1.0*TimeTot[k],Ttot[k]))

   if Ttot[noGOLD_ALL]>0 :
      print ("With NO reference solution")
      F = 100.0*Ttot[noGOLD_FITTING]/Ttot[noGOLD_ALL]
      print ("   Fitting: {:.2f}% ({}/{})".format(F,Ttot[noGOLD_FITTING],Ttot[noGOLD_ALL]))   
      for k in [noGOLD_FITTING,noGOLD_ALL] :
         N = prefix[k]
         DR = 1.0*SRcost[k]/Ttot[k] if Ttot[k]>0 else 0
         print ("   {}_AvgCostSol: {:.2f} ({}/{})".format(N,DR,SRcost[k],Ttot[k]))
         DG = 1.0*SGcost[k]/Ttot[k] if Ttot[k]>0 else 0
         print ("   {}_AvgCostRef: {:.2f} ({}/{})".format(N,DG,SGcost[k],Ttot[k]))
         DF = 1.0*(SRcost[k]-SGcost[k])/Ttot[k]
         TT = 1.0*TimeTot[k]/Ttot[k]
         print ("   {}_AvgTraceTime: {:.6f} ({:.1f}/{})".format(N,TT,1.0*TimeTot[k],Ttot[k]))
         print ("   {}_TotalTime: {:.1f} ({} traces)".format(N,1.0*TimeTot[k],Ttot[k]))

# -------------------------------

def separate(s) :
   x = s.rfind("]")
   m = s[0:x+1]  # event mark (L,L/M) in output 
   t = s[x+1:] # task name in output
   return m,t

   
# -------------------------------
# MAIN program

if len(sys.argv)<3 :
   print('Usage: ',sys.argv[0],"golddir outdir")
   exit()
   
golddir = sys.argv[1]
outdir = sys.argv[2]

Tok = [0,0,0,0]  # Traces ok (as a whole) in total, and only fitting
Ttot = [0,0,0,0] # Traces in total, and only fitting
Etot = [0,0,0,0] # Events in total, and only fitting
EokLM = [0,0,0,0] # Events ok (LM in output and gold) in total, and only fitting
EokL = [0,0,0,0] # Events ok (L in output and gold) in total, and only fitting
Epred = [0,0,0,0] # events predicted (LM or L in output) in total, and only fitting
Eexp = [0,0,0,0] # events expected (LM in gold) in total, and only fitting
Icost = [0,0,0,0] # traces with identical cost in total, and only fitting
SGcost = [0,0,0,0] # sum of costs for gold 
SRcost = [0,0,0,0] # sum of costs for result
TimeTot = [0,0,0,0] # time used in total

for f in sorted(os.listdir(outdir)) :
   error = False

   Tokf = [0,0,0,0] # Traces ok (as a whole) in current file: all / only fitting
   Ttotf = [0,0,0,0] # Traces in total in current file: all / only fitting
   Etotf = [0,0,0,0] # Events in total in current file: all / only fitting
   EokLMf = [0,0,0,0] # Events ok (LM in output and gold) in current file: all / only fitting
   EokLf = [0,0,0,0] # Events ok (L in output and gold) in current file: all / only fitting
   Epredf = [0,0,0,0] # events predicted (LM or L in output) in current file: all / only fitting
   Eexpf = [0,0,0,0] # events expected (LM in gold) in current file: all / only fitting
   Icostf = [0,0,0,0] # traces with identical cost in current file: all / only fitting
   SGcostf = [0,0,0,0] # sum of cost for gold in current file: all / only fitting
   SRcostf = [0,0,0,0] # sum of cost for result in current file: all / only fitting
   TimeTotf = [0,0,0,0] # time used in this file
    
   fname = f.replace(".out",".gold")
   print ("------------------------------------------")

   try :
      fout = open(outdir+"/"+f)
      fgold = open(golddir+"/"+fname)
   except:
      continue

   lout = fout.readline()
   lgold = fgold.readline()   
   while lout!='' and lgold!='' :
      (idout,aligout,fitout,time) = lout.split()
      time = float(time)
      fitting = (fitout=="FITTING")
      (idgold,aliggold) = lgold.split()

      if idgold < idout :
         # unsolved case, skip gold. (Should not happen, since we always produce an output)
         print(f,"- UNSOLVED instance", idgold)
         # next
         lgold = fgold.readline()
         
      elif idout < idgold :
         # no gold solution for this trace. Count as "no-gold"
         cost = aligout.count("[M-REAL]") + aligout.count("[L]")
         SRcostf[noGOLD_ALL] += cost
         Ttotf[noGOLD_ALL] += 1
         TimeTotf[noGOLD_ALL] += time
         if fitting :
            SRcostf[noGOLD_FITTING] += cost
            Ttotf[noGOLD_FITTING] += 1
            TimeTotf[noGOLD_FITTING] += time
         # next
         lout = fout.readline()
         
      else :
         # IDs match, compare traces
         
         evout = aligout.split("|")   # events in output
         evgold = aliggold.split("|") # events in gold
         acevout = len(evout) - aligout.count("[M-REAL]"); # actual events (i.e. not inserted) in output
         acevgold = len(evgold) - aliggold.count("[M-REAL]"); # actual events (i.e. not inserted) in gold
         if acevout!=acevgold :
            print(f,"- Length mismatch",idout,idgold,"\n", aligout,"\n", aliggold)
            error = True
            break

         iout = 0
         igold = 0
         while iout<len(evout) and igold<len(evgold) :
            ## advance iout skipping insertions
            while iout<len(evout) and evout[iout].count("[M-REAL]")>0 : iout += 1
            if iout==len(evout) : (mout,tout)==("EOT","EOT")
            else : (mout,tout) = separate(evout[iout])

            ## advance igold skipping insertions
            while igold<len(evgold) and evgold[igold].count("[M-REAL]")>0 : igold += 1
            if igold==len(evgold) : (mgold,tgold)==("EOT","EOT")
            else : (mgold,tgold) = separate(evgold[igold])

            if tout!=tgold :
               print(f,"- Task mismatch",idout,idgold,"\n", tout,"\n", tgold)
               error = True
               break

            Etotf[GOLD_ALL] += 1
            if mout=="[L/M]" and mgold=="[L/M]" : EokLMf[GOLD_ALL] += 1
            if mout=="[L]" and mgold=="[L]" : EokLf[GOLD_ALL] += 1
            if mout=="[L/M]" : Epredf[GOLD_ALL] += 1
            if mgold=="[L/M]" : Eexpf[GOLD_ALL] += 1
            if fitting :
               Etotf[GOLD_FITTING] += 1
               if mout=="[L/M]" and mgold=="[L/M]" : EokLMf[GOLD_FITTING] += 1
               if mout=="[L]" and mgold=="[L]" : EokLf[GOLD_FITTING] += 1
               if mout=="[L/M]" : Epredf[GOLD_FITTING] += 1
               if mgold=="[L/M]" : Eexpf[GOLD_FITTING] += 1

            iout += 1
            igold += 1
         # end while on events
         
         if not error :
            costout = aligout.count("[M-REAL]") + aligout.count("[L]")
            costgold = aliggold.count("[M-REAL]") + aliggold.count("[L]")

            Ttotf[GOLD_ALL] += 1
            if aligout==aliggold : Tokf[GOLD_ALL] += 1
            if costout==costgold : Icostf[GOLD_ALL] += 1
            SRcostf[GOLD_ALL] += costout
            SGcostf[GOLD_ALL] += costgold
            TimeTotf[GOLD_ALL] += time
            if fitting :
               Ttotf[GOLD_FITTING] += 1
               if aligout==aliggold : Tokf[GOLD_FITTING] += 1
               if costout==costgold : Icostf[GOLD_FITTING] += 1
               SRcostf[GOLD_FITTING] += costout
               SGcostf[GOLD_FITTING] += costgold
               TimeTotf[GOLD_FITTING] += time

         lout = fout.readline()
         lgold = fgold.readline()   

      # end case ids match

   #end for each trace
         
   if not error :
      stats (f,Tokf,Ttotf,Etotf,EokLMf,EokLf,Epredf,Eexpf,Icostf,SGcostf,SRcostf,TimeTotf)
      for k in [GOLD_ALL,GOLD_FITTING,noGOLD_ALL,noGOLD_FITTING] :
         Tok[k] += Tokf[k]
         Ttot[k] += Ttotf[k]
         EokLM[k] += EokLMf[k]
         EokL[k] += EokLf[k]
         Epred[k] += Epredf[k]
         Eexp[k] += Eexpf[k]
         Etot[k] += Etotf[k]
         Icost[k] += Icostf[k]
         SRcost[k] += SRcostf[k]
         SGcost[k] += SGcostf[k]
         TimeTot[k] += TimeTotf[k]

# end for each file
    
print ("...........................................")
stats ("TOTAL",Tok,Ttot,Etot,EokLM,EokL,Epred,Eexp,Icost,SGcost,SRcost,TimeTot)


