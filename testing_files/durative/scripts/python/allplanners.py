import matplotlib
matplotlib.use('SVG')

import numpy as np
import math
import matplotlib.pyplot as pyplot
from array import array
from matplotlib.backends.backend_pdf import PdfPages

PLANNERS = ["itsat", "optic", "tfd", "yahsp"]
DAES = ["10", "128", "1024", "2014", "4067"]
PARAMETERS = ["makespan", "memory", "real_runtime", "valid"]


limitm = []
limitme = []
limitt = []
limituS=[]
limitlS=[]

limituX = []
limitlX = []

g=-1

popmer_makespan = []
popmer_valid = []
popmer_time = []
popmer_rel_time = []
popmer_time_new = []
popmer_rel_time_new = []
popmer_memory = []
popmer_rel_memory = []

vhpop_seq_makespan = []
vhpop_uni_makespan = []

vhpop_seq_valid = []
vhpop_uni_valid = []

vhpop_uni_time = []
vhpop_uni_memory = []

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/makespan','r') as filei:
  for line in filei:
     makespans = line.split(";")
     value=makespans[len(makespans)-2]
     popmer_makespan.append(float(value))

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_sequence_30min_8GB/makespan','r') as filei:
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] 
    vhpop_seq_makespan.append(float(value))

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_full_30min_8GB/makespan','r') as filei:
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] 
    vhpop_uni_makespan.append(float(value))

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_full_30min_8GB/memory_6GB','r') as filei:
   for line in filei:
    makespans = line.split(" ")
    if(len(makespans) > 2):
      value=makespans[2] 
      vhpop_uni_memory.append(float(value)/1024.0)


with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_full_30min_8GB/real_runtime','r') as filei:
   for line in filei:
    makespans = line.split("\n")
    if(makespans[0] == " "):
      continue;
    else:
      value=makespans[0] 
      vhpop_uni_time.append(float(value))

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if(value=="1"):
       popmer_valid.append(x)
    else:
       del popmer_makespan[x-1-invalid]
       invalid = invalid+1
    x=x+1

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_sequence_30min_8GB/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if(value=="1"):
       vhpop_seq_valid.append(x)
    else:
       del vhpop_seq_makespan[x-1-invalid]
       invalid = invalid+1
    x=x+1

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/ipc3_vhpop_full_30min_8GB/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if(value=="1"):
       vhpop_uni_valid.append(x)
    else:
       del vhpop_uni_makespan[x-1-invalid]
       invalid = invalid+1
    x=x+1


vhpop_seq_score = []
vhpop_uni_score = []
popmer_score = []
lastIndex=0
for i in range(0,len(popmer_valid)): 
    popmer_score.append(1) 
    if(lastIndex >= len(vhpop_seq_valid)):
         vhpop_seq_score.append(0)
    else:
      if(popmer_valid[i] == vhpop_seq_valid[lastIndex]):
         vhpop_seq_score.append(float(popmer_makespan[i])/float(vhpop_seq_makespan[lastIndex]))
         lastIndex = lastIndex+1
      elif(popmer_valid[i] < vhpop_seq_valid[lastIndex]): #my merge found sol, but the other not
         vhpop_seq_score.append(0)
lastIndex=0
for i in range(0,len(popmer_valid)):  
    if(lastIndex >= len(vhpop_uni_valid)):
         vhpop_uni_score.append(0)
    else:
      if(popmer_valid[i] == vhpop_uni_valid[lastIndex]):
         vhpop_uni_score.append(float(popmer_makespan[i])/float(vhpop_uni_makespan[lastIndex]))
         lastIndex = lastIndex+1
      elif(popmer_valid[i] < vhpop_uni_valid[lastIndex]): #my merge found sol, but the other not
         vhpop_uni_score.append(0)

#reading in popmer new time------------
with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/new_time/POPMER_IPCVHPOP_30min_8GB/real_runtime','r') as filei:
  for line in filei:
     makespans = line.split("\n")
     if(makespans[0] != " "):
       value=makespans[0]
       popmer_time_new.append(float(value))
       popmer_rel_time_new.append(1)

#reading in popmer time------------
with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/real_runtime','r') as filei:
  for line in filei:
     makespans = line.split("\n")
     if(makespans[0] != " "):
       value=makespans[0]
       popmer_time.append(float(value))
       popmer_rel_time.append(1)

#reading in popmer memory------------
with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/POPMER_IPCVHPOP_30min_8GB/memory','r') as filei:
  for line in filei:
     makespans = line.split(" ")
     if(makespans[0] != ""):
       value=makespans[2]
       popmer_memory.append(float(value)/1024.0)
       popmer_rel_memory.append(1)


limitlX.append(min(popmer_valid)-1)
limituX.append(max(popmer_valid)+1)

#maybe it could work to make list of lists

itsat_makespan = []
optic_makespan = []
tfd_makespan = []
yahsp_makespan = []

itsat_valid = []
optic_valid = []
tfd_valid = []
yahsp_valid = []

itsat_score = []
optic_score = []
tfd_score = []
yahsp_score = []

tfd_amount = []
yahsp_amount = []
itsat_amount = []

tfd_time = []
tfd_x = []
yahsp_time = []
yahsp_x = []
optic_time = []
optic_x = []
itsat_time = []
itsat_x = []

tfd_memory = []
tfd_x2 = []
yahsp_memory = []
yahsp_x2 = []
optic_memory = []
optic_x2 = []
itsat_memory = []
itsat_x2 = []

tfd_one_score = []
yahsp_one_score = []
tfd_one_time = []
yahsp_one_time = []
tfd_one_memory = []
yahsp_one_memory = []


with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/optic_30min_6GB/real_runtime','r') as filei:
  it = 0
  for line in filei:
    makespans = line.split(" ")
    if (makespans[1] != "\n"):
       optic_time.append(float(makespans[1]))
       it = it +1 
       optic_x.append(it)

with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/optic_30min_6GB/memory','r') as filei:
  it = 0
  for line in filei:
    makespans = line.split(" ")
    if (makespans[1] != "\n"):
       optic_memory.append(float(makespans[3])/1024.0)
       it = it +1 
       optic_x2.append(it)



#TODO change here for other problems
problemID = 4



for plan in PLANNERS:
 
  g=g+1
  
  #time-----------------
  it=1
  if((plan == "tfd") or (plan=="yahsp") or (plan == "itsat")):
   
   with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/'+plan+'_30min_6GB/sep_time','r') as filei:
 

    for line in filei:
     temp = []
     makespans = line.split(";")
     if(plan == "tfd"):
       tfd_amount.append(len(makespans))
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])
         
         if(value <= 0.1):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         tfd_time.append(value)
         tfd_x.append(it)
         
         
     elif(plan == "yahsp"):
       yahsp_amount.append(len(makespans))
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])

         if(value <= 0.1):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         yahsp_time.append(value)
         yahsp_x.append(it)

     elif(plan == "itsat"):
       itsat_amount.append(len(makespans))
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])

         if(value <= 0.1):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         itsat_time.append(value)
         itsat_x.append(it)

     if(it == problemID):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])

         if(plan == "tfd"):     
           if(value <= 0.1):
             if(len(tfd_one_time) > 0): #there is an item
               value = tfd_one_time[len(tfd_one_time)-1]
             else:
               value = -1

           tfd_one_time.append(value)

         elif(plan == "yahsp"):
           if(value <= 0.1):
             if(len(yahsp_one_time) > 0): #there is an item
               value = yahsp_one_time[len(yahsp_one_time)-1]
             else:
               value = -1

           yahsp_one_time.append(value)

     it = it +1

  #memory-----------------
  it=1
  if((plan == "tfd") or (plan=="yahsp") or (plan == "itsat")):
   
   with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/'+plan+'_30min_6GB/sep_memory','r') as filei:
    for line in filei:
     temp = []
     makespans = line.split(";")
     if(plan == "tfd"):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])/1024

         if(value < 0):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         tfd_memory.append(value)
         tfd_x2.append(it)
       
         
     elif(plan == "yahsp"):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])/1024
         if(value < 0):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         yahsp_memory.append(value)
         yahsp_x2.append(it)

     elif(plan == "itsat"):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])/1024
         if(value < 0):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
         temp.append(value)
         itsat_memory.append(value)
         itsat_x2.append(it)

     if(it == problemID):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])/1024.0
         
         if(plan == "tfd"):
           if(value < 0):         
             if(len(tfd_one_memory) > 0): #there is an item
               value = tfd_one_memory[len(tfd_one_memory)-1]
             else:
               value = -1
               print "happens"+plan

           tfd_one_memory.append(value)

         elif(plan == "yahsp"):
           if(value < 0):
             if(len(yahsp_one_memory) > 0): #there is an item
               value = yahsp_one_memory[len(yahsp_one_memory)-1]
             else:
               value = -1
               print "happens"+plan

           yahsp_one_memory.append(value)

     it = it +1

  
  it =1
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/'+plan+'_30min_6GB/makespan','r') as filei:
    for line in filei:
     makespans = line.split(";")
     value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
     if(plan == "itsat"):
       itsat_makespan.append(float(value))
     elif(plan == "optic"):
       optic_makespan.append(float(value))
     elif(plan == "tfd"):
       tfd_makespan.append(float(value))
     elif(plan == "yahsp"):
       yahsp_makespan.append(float(value))

     if(it == problemID):
       for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])
         if(plan == "tfd"):
             tfd_one_score.append(popmer_makespan[it-1]/value)
         elif(plan == "yahsp"):
             yahsp_one_score.append(popmer_makespan[it-1]/value)

     it = it+1



  #VALID----------
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/'+plan+'_30min_6GB/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if(value=="1"):
      if(plan == "itsat"):
       itsat_valid.append(x)
      elif(plan == "optic"):
       optic_valid.append(x)
      elif(plan == "tfd"):
       tfd_valid.append(x)
      elif(plan == "yahsp"):
       yahsp_valid.append(x)
    else:
      
      if(plan == "itsat"):
       del itsat_makespan[x-1-invalid]
      elif(plan == "optic"):
       del optic_makespan[x-1-invalid]
      elif(plan == "tfd"):
       del tfd_makespan[x-1-invalid]
      elif(plan == "yahsp"):
       del yahsp_makespan[x-1-invalid]
      invalid = invalid+1
    x=x+1



  #ICPScore criterion

  lastIndex=0
  for i in range(0,len(popmer_valid)):  
    if(plan == "itsat"):
       if(lastIndex >= len(itsat_valid)):
         itsat_score.append(0)
       else:
         if(popmer_valid[i] == itsat_valid[lastIndex]):
           itsat_score.append(float(popmer_makespan[i])/float(itsat_makespan[lastIndex]))
           lastIndex = lastIndex+1
         elif(popmer_valid[i] < itsat_valid[lastIndex]): #my merge found sol, but the other not
           itsat_score.append(0)
    elif(plan == "optic"):
       if(lastIndex >= len(optic_valid)):
         optic_score.append(0)
       else:
         if(popmer_valid[i] == optic_valid[lastIndex]):
           optic_score.append(float(popmer_makespan[i])/float(optic_makespan[lastIndex]))
           lastIndex = lastIndex+1
         elif(popmer_valid[i] < optic_valid[lastIndex]): #my merge found sol, but the other not
           optic_score.append(0)
    elif(plan == "tfd"):
       if(lastIndex >= len(tfd_valid)):
         tfd_score.append(0)
       else:
         if(popmer_valid[i] == tfd_valid[lastIndex]):
           tfd_score.append(float(popmer_makespan[i])/float(tfd_makespan[lastIndex]))
           lastIndex = lastIndex+1
         elif(popmer_valid[i] < tfd_valid[lastIndex]): #my merge found sol, but the other not
           tfd_score.append(0)
    elif(plan == "yahsp"):
       if(lastIndex >= len(yahsp_valid)):
         yahsp_score.append(0)
       else:
         if(popmer_valid[i] == yahsp_valid[lastIndex]):
           yahsp_score.append(float(popmer_makespan[i])/float(yahsp_makespan[lastIndex]))
           lastIndex = lastIndex+1
         elif(popmer_valid[i] < yahsp_valid[lastIndex]): #my merge found sol, but the other not
           yahsp_score.append(0)


dae_makespan_b = [None]*23
dae_makespan_w = [None]*23
dae_score = []
dae_2014_score = []
dae_time = []
dae_x = []
dae_memory = []
dae_x2 = []

dae_one_memory = []
dae_one_time = []
dae_one_score = []


dae_score_time = [None]*23

for dae in DAES:  

  dae_amount = []
  dae_index = [None]*23
  it =1
  #if(dae == "2014"):
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/dae_30min_6GB_'+dae+'/sep_time','r') as filei:
    for line in filei:
     temp =[]
     makespans = line.split(";")

     dae_amount.append(len(makespans))
     for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])     
         
         if(value <= popmer_time[it-1]):
           dae_index[it-1] = i

         if(dae=="2014"):
           if(value <= 0.1):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
           temp.append(value)
           dae_time.append(value)
           dae_x.append(it)
     
     if(dae == "2014"):     
       if(it == problemID):
          for i in range(0,len(makespans)-1): #the last in \n
                value = float(makespans[i])
                if(value < 0):
                   if(len(dae_one_time) > 0): #there is an item
                      value = dae_one_time[len(dae_one_time)-1]
                   else:
                      value = -1
               
                dae_one_time.append(value)

     it = it +1

  it =1
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/dae_30min_6GB_'+dae+'/sep_memory','r') as filei:
    for line in filei:
     makespans = line.split(";")
     temp = []
     for i in range(0,len(makespans)-1): #the last in \n
         value = float(makespans[i])     
         
         if(dae=="2014"):
           value = value/1024
           if(value < 0):
             if(len(temp) > 0): #there is an item
               value = temp[len(temp)-1]
             else:
               continue #do not save
           temp.append(value)
           dae_memory.append(value)
           dae_x2.append(it)

     if(dae=="2014"):
       if(it == problemID):
          for i in range(0,len(makespans)-1): #the last in \n
                value = float(makespans[i])/1024.0
                if(value < 0):
                   if(len(dae_one_memory) > 0): #there is an item
                      value = dae_one_memory[len(dae_one_memory)-1]
                   else:
                      value = -1
               
                dae_one_memory.append(value)

     it = it +1

  it=0
  it2=1
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/driverlog/dae_30min_6GB_'+dae+'/makespan','r') as filei:
    for line in filei:
     makespans = line.split(";")
     if(len(makespans) != dae_amount[it]):
       raise ValueError('not same amount of solutions in makespan and time files')
     value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
     temp = float(value)

     if(dae == "2014"):
       dae_2014_score.append(temp)
       if(it2 == problemID):
         for i in range(0,len(makespans)-1): #the last in \n
           value = float(makespans[i])
           dae_one_score.append(popmer_makespan[it2-1]/value)

     if(dae_makespan_b[it] != None):
       if(dae_makespan_b[it] > temp):
         dae_makespan_b[it] = temp
     else:
       dae_makespan_b[it] = temp

     if(dae_makespan_w[it] != None):
       if(dae_makespan_w[it] < temp):
         dae_makespan_w[it] = temp
     else:
       dae_makespan_w[it] = temp

     temp = float(makespans[dae_index[it]])
     if(dae_score_time[it] != None):
       if(dae_score_time[it] > temp):
         dae_score_time[it] = temp
     else:
       dae_score_time[it] = temp 
     it=it+1
     it2=it2+1

  

#TODO maybe add valid if it doesnt find some problem


for i in range(0,len(popmer_valid)):  
   dae_score.append(float(popmer_makespan[i])/float(dae_makespan_b[i]))
   dae_2014_score[i] = float(popmer_makespan[i])/float(dae_2014_score[i])
   dae_score_time[i] = float(popmer_makespan[i])/float(dae_score_time[i])
   
        

  
         
#limitm.append(math.ceil(max([max(mymerge),max(uni),max(naive)])/100)*110.0)      
#limitme.append(math.ceil(max([max(memorys),max(memorym),max(memoryu)])/100)*110.0) 
#limitt.append(math.ceil(max([max(times),max(timem),max(timeu)])/10)*11.0)   

#print limitm, limitme, limitt

limituS.append(math.ceil(max([max(itsat_score),max(optic_score),max(tfd_score),max(yahsp_score),max(dae_score), max(vhpop_seq_score), max(vhpop_uni_score)])*10)/10.0+0.1)
limitlS.append(math.floor(min([min(itsat_score),min(optic_score),min(tfd_score),min(yahsp_score),min(dae_score), min(vhpop_seq_score)])*10)/10.0-0.1) #vhpop_uni_score is intentionally left out due zeros


#GRAPHS------------------


#IPCscore---------
fig = pyplot.figure(1)
ax = fig.add_subplot(111)
#ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('RIPCscore',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], limitlS[0],limituS[0]])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

pyplot.plot(popmer_valid,popmer_score,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='None', label='POPMER$_{IPC3-VHPOP}$')
pyplot.plot(popmer_valid,vhpop_seq_score,color='#fca771', linestyle='None', markeredgecolor='#fca771', marker='s', label='S-IPC3-VHPOP')
pyplot.plot(popmer_valid,tfd_score,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='TFD')
pyplot.plot(popmer_valid,yahsp_score,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='YAHSP')
pyplot.plot(popmer_valid,dae_score,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='DAE$_{YAHSP}$')
#pyplot.plot(popmer_valid,dae_score_time,color='None', linestyle='None', markeredgecolor='black', marker='D', label='DAE$_{YAHSP}$(time)')

pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/g-RIPCscore1.svg',bbox_inches='tight',  format = 'svg')

#print "scores"
#print "popmer", sum(popmer_score)
#print "vhpop_seq", sum(vhpop_seq_score)
#print "tfd", sum(tfd_score)
#print "yahsp", sum(yahsp_score)
#print "dae", sum(dae_score)
#print "dae_time", sum(dae_score_time)
#print "vhpop_uni", sum(vhpop_uni_score)
#print "itsat", sum(itsat_score)
#print "optic", sum(optic_score)



better=0
worse=0
diff1=[]
diff2=[]

#TODO zde jsem skoncila muzu pouzit scores
#for i in range (0,len(popmer_valid)):
#    diff_vhpop_seq.append(vhpop_seq_makespan_b[i] - popmer_makespan[i])

 

#print "-------"
#print "vhpop_seq"
#print "tfd"
#print "yahsp"
#print "dae"
#print "vhpop_uni"
#print "itsat"
#print "optic"


#print "----------50"
print round(np.percentile(np.array(vhpop_seq_score), 50)*100 - 100)
print round(np.percentile(np.array(tfd_score), 50)*100 - 100)
print round(np.percentile(np.array(yahsp_score), 50)*100 - 100)
print round(np.percentile(np.array(dae_score), 50)*100 - 100)
print round(np.percentile(np.array(vhpop_uni_score), 50)*100 - 100)
print round(np.percentile(np.array(itsat_score), 50)*100 - 100)
print round(np.percentile(np.array(optic_score), 50)*100 - 100)

#print "-----75"
print round(np.percentile(np.array(vhpop_seq_score), 75)*100 - 100)
print round(np.percentile(np.array(tfd_score), 75)*100 - 100)
print round(np.percentile(np.array(yahsp_score), 75)*100 - 100)
print round(np.percentile(np.array(dae_score), 75)*100 - 100)
print round(np.percentile(np.array(vhpop_uni_score), 75)*100 - 100)
print round(np.percentile(np.array(itsat_score), 75)*100 - 100)
print round(np.percentile(np.array(optic_score), 75)*100 - 100)

#print "-----50"
#print round(sum(vhpop_uni_score)/23*100-100)
#print round(sum(optic_score)/23*100-100)
#print round(sum(itsat_score)/23*100-100)

#IPCscore2------------
fig = pyplot.figure(2)
ax = fig.add_subplot(111)
#ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('RIPCscore',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], limitlS[0],limituS[0]])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

pyplot.plot(popmer_valid,popmer_score,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='None', label='POPMER$_{IPC3-VHPOP}$')
pyplot.plot(popmer_valid,vhpop_seq_score,color='#fca771', linestyle='None', markeredgecolor='#fca771', marker='s', label='S-IPC3-VHPOP')
pyplot.plot(popmer_valid,vhpop_uni_score,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-IPC3-VHPOP')
pyplot.plot(popmer_valid,itsat_score,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='*', label='ITSAT')
pyplot.plot(popmer_valid,optic_score,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='v', label='OPTIC')

pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/g-RIPCscore2.svg',bbox_inches='tight',  format = 'svg')




#TIME------------------------------------
time_u = 1.1*(math.ceil(max([max(tfd_time),max(yahsp_time), max(dae_time)])/100)*100)
time_l = 0.9*(math.floor(min([min(tfd_time),min(yahsp_time), min(dae_time)])*100)/100)
fig = pyplot.figure(3)
ax = fig.add_subplot(111)
ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('runtime [s]',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], 0.1, 10000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

max_time = [1800]*26
max_x = range(0,26)

print popmer_time_new
print range(1,24)
pyplot.plot(max_x,max_time,color='red', linestyle='-', label='maximal allowed time')
pyplot.plot(dae_x,dae_time,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='DAE$_{YAHSP}$(2014)')
pyplot.plot(tfd_x,tfd_time,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='TFD')
pyplot.plot(yahsp_x,yahsp_time,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='YAHSP')
pyplot.plot(popmer_valid,popmer_time,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')
pyplot.plot(range(1,24),popmer_time_new,color='red', linestyle='-', markeredgecolor='red', marker='o', label='POPMER$_{IPC3-VHPOP}$')

pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/time.svg',bbox_inches='tight',  format = 'svg')



#MEMORY------------------------------------
memory_u = (math.ceil(max([max(tfd_memory),max(yahsp_memory), max(dae_memory)])/10)*10)
memory_l = (math.floor(min([min(tfd_memory),min(yahsp_memory), min(dae_memory)])*1000)/1000)

fig = pyplot.figure(4)
ax = fig.add_subplot(111)
ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('memory [MB]',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], 1,10000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

max_memory = [6*1024]*26

pyplot.plot(max_x,max_memory,color='red', linestyle='-', label='maximal allowed memory')
pyplot.plot(dae_x2,dae_memory,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='DAE$_{YAHSP}$(2014)')
pyplot.plot(tfd_x2,tfd_memory,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='TFD')
pyplot.plot(yahsp_x2,yahsp_memory,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='YAHSP')
pyplot.plot(popmer_valid,popmer_memory,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')
pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/memory.svg',bbox_inches='tight',  format = 'svg')

#----------MEMORY vs SCORE

print max([max(dae_one_memory), max(tfd_one_memory), max(yahsp_one_memory),popmer_memory[problemID-1]])
mem_limit=100
#4, 14, 22, 23
#100, 3000, 3000, 3000
#position of I
#60, 1000, 1000, 1000

fig = pyplot.figure(5)
ax = fig.add_subplot(111)
ax.set_xscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('memory [MB]',fontsize=20)
pyplot.ylabel('RIPCscore',fontsize=20)

pyplot.axis([10,mem_limit, 0.5,1.3])
pyplot.grid(b=True,which='major')

#print "-------memory vs score--------"
#print len(dae_one_memory),len(dae_one_score)

border1_x = [0, mem_limit]
border1_y = [1, 1]

border2_x = [popmer_memory[problemID-1],popmer_memory[problemID-1]]
border2_y = [0.5,1.3]

popmer_border_x = [popmer_memory[problemID-1]]
popmer_border_y = [1]

#'$...$'

pyplot.plot(border1_x,border1_y,color='#fca771', linestyle='-', markeredgecolor='#fca771', marker='None')
pyplot.plot(border2_x,border2_y,color='#fca771', linestyle='-', markeredgecolor='#fca771', marker='None')
pyplot.plot(13,1.25,color='black', linestyle='None', markeredgecolor='black', markersize=13, marker='$I$')
pyplot.plot(60,1.25,color='black', linestyle='None', markeredgecolor='black', markersize=18, marker='$II$')
pyplot.plot(13,0.55,color='black', linestyle='None', markeredgecolor='black', markersize=23, marker='$III$')
pyplot.plot(60,0.55,color='black', linestyle='None', markeredgecolor='black', markersize=20, marker='$IV$')

pyplot.plot(dae_one_memory,dae_one_score,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='DAE$_{YAHSP}$(2014)')
pyplot.plot(tfd_one_memory,tfd_one_score,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='TFD')
pyplot.plot(yahsp_one_memory,yahsp_one_score,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='YAHSP')
pyplot.plot(popmer_border_x,popmer_border_y,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')
pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)


pyplot.savefig('driverlog/memoryvscore'+str(problemID)+'.svg',bbox_inches='tight',  format = 'svg')



#----------TIME vs SCORE
#4, 14, 22, 23
#100, 100, 1000, 1000
#position of I
#60, 60, 600, 600

print max([max(dae_one_time), max(tfd_one_time), max(yahsp_one_time),popmer_time[problemID-1]])
mem_limit=100
fig = pyplot.figure(6)
ax = fig.add_subplot(111)
ax.set_xscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('runtime [s]',fontsize=20)
pyplot.ylabel('RIPCscore',fontsize=20)

pyplot.axis([0.01,mem_limit, 0.5,1.3])
pyplot.grid(b=True,which='major')

#print "-------memory vs score--------"
#print len(dae_one_memory),len(dae_one_score)

border1_x = [0, mem_limit]
border1_y = [1, 1]

border2_x = [popmer_time[problemID-1],popmer_time[problemID-1]]
border2_y = [0.5,1.3]

popmer_border_x = [popmer_time[problemID-1]]
popmer_border_y = [1]

#'$...$'

pyplot.plot(border1_x,border1_y,color='#fca771', linestyle='-', markeredgecolor='#fca771', marker='None')
pyplot.plot(border2_x,border2_y,color='#fca771', linestyle='-', markeredgecolor='#fca771', marker='None')
pyplot.plot(0.03,1.25,color='black', linestyle='None', markeredgecolor='black', markersize=13, marker='$I$')
pyplot.plot(60,1.25,color='black', linestyle='None', markeredgecolor='black', markersize=18, marker='$II$')
pyplot.plot(0.03,0.55,color='black', linestyle='None', markeredgecolor='black', markersize=23, marker='$III$')
pyplot.plot(60,0.55,color='black', linestyle='None', markeredgecolor='black', markersize=20, marker='$IV$')

pyplot.plot(dae_one_time,dae_one_score,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='DAE$_{YAHSP}$(2014)')
pyplot.plot(tfd_one_time,tfd_one_score,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='TFD')
pyplot.plot(yahsp_one_time,yahsp_one_score,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='YAHSP')
pyplot.plot(popmer_border_x,popmer_border_y,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')
pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)


pyplot.savefig('driverlog/timevscore'+str(problemID)+'.svg',bbox_inches='tight',  format = 'svg')



#-----------------
#TIME2------------------------------------
time_u = 1.1*(math.ceil(max([max(tfd_time),max(yahsp_time), max(dae_time)])/100)*100)
time_l = 0.9*(math.floor(min([min(tfd_time),min(yahsp_time), min(dae_time)])*100)/100)
fig = pyplot.figure(7)
ax = fig.add_subplot(111)
ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('runtime [s]',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], 0.1, 10000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

max_time = [1800]*26
max_x = range(0,26)

pyplot.plot(max_x,max_time,color='red', linestyle='-', label='maximal allowed time')
pyplot.plot(popmer_valid,vhpop_uni_time,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-IPC3-VHPOP')
pyplot.plot(itsat_x,itsat_time,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='*', label='ITSAT')
pyplot.plot(optic_x,optic_time,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='v', label='OPTIC')
pyplot.plot(popmer_valid,popmer_time,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')

pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/time2.svg',bbox_inches='tight',  format = 'svg')


#MEMORY2------------------------------------
memory_u = (math.ceil(max([max(tfd_memory),max(yahsp_memory), max(dae_memory)])/10)*10)
memory_l = (math.floor(min([min(tfd_memory),min(yahsp_memory), min(dae_memory)])*1000)/1000)

fig = pyplot.figure(8)
ax = fig.add_subplot(111)
ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)

pyplot.rc('text', usetex=True)
pyplot.rc('font', family='serif', size='20')
pyplot.rc('legend', fontsize= '15', numpoints='1')
   
pyplot.xlabel('number of goal literals',fontsize=20)
pyplot.ylabel('memory [MB]',fontsize=20)

pyplot.axis([limitlX[0],limituX[0], 1,10000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

max_memory = [6*1024]*26

pyplot.plot(max_x,max_memory,color='red', linestyle='-', label='maximal allowed time')
pyplot.plot(popmer_valid,vhpop_uni_memory,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-IPC3-VHPOP')
pyplot.plot(itsat_x2,itsat_memory,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='*', label='ITSAT')
pyplot.plot(optic_x,optic_memory,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='v', label='OPTIC')
pyplot.plot(popmer_valid,popmer_memory,color='#1b2c3f', linestyle='-', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{IPC3-VHPOP}$')

pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('driverlog/memory2.svg',bbox_inches='tight',  format = 'svg')

