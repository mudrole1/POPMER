import matplotlib
matplotlib.use('SVG')

import numpy as np
import math
import matplotlib.pyplot as pyplot
from array import array
from matplotlib.backends.backend_pdf import PdfPages

PLANNERS = ["itsat", "optic", "vhpop"]
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
popmer_memory = []
popmer_rel_memory = []

vhpop_seq_makespan = []
vhpop_uni_makespan = []

vhpop_seq_valid = []
vhpop_uni_valid = []



vhpop_seq_score = []
vhpop_uni_score = []
popmer_score = []
lastIndex=0



"02_0", "02_1", "03_0", "03_1", "04_0", "04_1", "05_0", "05_1", "10_0", "15_0", "20_0", "25_0", "30_0", "35_0", "40_0", "45_0", "50_0"

"03", "04", "05", "06", "07", "08", "09", "10", "100", "20", "30", "40", "50", "60", "70", "80", "90", 

popmer_valid = [2,3,4,5,10,15,20,25,30,35,40,45,50]


#reading in popmer time------------
it=2
it2=0
temp_value = 0
with open('/data/Phd_final/Phd_evaluation/durative/results/tms/popmer_30min_6GB/real_runtime','r') as filei:
  for line in filei:
     makespans = line.split(" ")
     if(makespans[0] != ""):
       value=makespans[1]
       if(it <=5):
         if(it2 == 0):
           temp_value = float(value)
           it2=it2+1
          
         else:
           popmer_time.append((float(value)+temp_value)/2)
           it = it+1
           it2 = 0
           temp_value = 0
       else:
         popmer_time.append(float(value))


#reading in popmer memory------------
it=2
it2=0
temp_value = 0
with open('/data/Phd_final/Phd_evaluation/durative/results/tms/popmer_30min_6GB/memory','r') as filei:
  for line in filei:
     makespans = line.split(" ")
     if(makespans[0] != ""):
       value=makespans[3]
       if(it <=5):
         if(it2 == 0):
           temp_value = float(value)
           it2=it2+1
         else:
           popmer_memory.append((float(value)+temp_value)/2/1024)
           it = it+1
           it2 = 0
           temp_value = 0
       else:
         popmer_memory.append(float(value)/1024.0)


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

tfd_time = []
tfd_x = []
yahsp_time = []
yahsp_x = []
itsat_time = []
optic_time = []
vhpop_time =[]

tfd_memory = []
tfd_x2 = []
yahsp_memory = []
yahsp_x2 = []
itsat_memory = []
optic_memory = []
vhpop_memory = []

tfd_one_score = []
yahsp_one_score = []
tfd_one_time = []
yahsp_one_time = []
tfd_one_memory = []
yahsp_one_memory = []

#TODO change here for other problems
problemID = 4


for plan in PLANNERS:
   
   it=2
   it2=0
   temp_value = 0
   hundred_value = 0
   with open('/data/Phd_final/Phd_evaluation/durative/results/tms/'+plan+'_30min_6GB/real_runtime','r') as filei:
 

    for line in filei:
     temp = []
     makespans = line.split(" ")
     if(makespans[0]!=""):
         print plan,makespans[0],"x"
         index = math.ceil(float(makespans[0])/2)  #03/2
         value = float(makespans[1])
         if (index == 50): #save this in the end
           hundred_value = value
           continue
         if(it <=5):
           if(it2 == 0):
             temp_value = value
             it2=it2+1
          
           else:
             if(plan == "itsat"):
               itsat_time.append((float(value)+temp_value)/2)
             if(plan == "optic"):
               optic_time.append((float(value)+temp_value)/2)
             if(plan == "vhpop"):
               vhpop_time.append((float(value)+temp_value)/2)
             it = it+1
             it2 = 0
             temp_value = 0
         else:
           if(plan == "itsat"):
             itsat_time.append(float(value))
           if(plan == "optic"):
             optic_time.append(float(value))

    if(plan == "itsat"):
      itsat_time.append(hundred_value)
    if(plan == "optic"):
      optic_time.append(hundred_value)

   #memory-----------------
  
   it=2
   it2=0
   temp_value = 0
   hundred_value = 0

   with open('/data/Phd_final/Phd_evaluation/durative/results/tms/'+plan+'_30min_6GB/memory','r') as filei:
 

    for line in filei:
     temp = []
     makespans = line.split(" ")
     if(makespans[0]!=""):
         index = math.ceil(float(makespans[0])/2)  #03/2
         value = float(makespans[1])
         if (index == 50): #save this in the end
           hundred_value = value
           continue
         if(it <=5):
           if(it2 == 0):
             temp_value = value
             it2=it2+1
          
           else:
             if(plan == "itsat"):
               itsat_memory.append((float(value)+temp_value)/2/1024)
             if(plan == "optic"):
               optic_memory.append((float(value)+temp_value)/2/1024)
             if(plan == "vhpop"):
               vhpop_memory.append((float(value)+temp_value)/2/1024)
             it = it+1
             it2 = 0
             temp_value = 0
         else:
           if(plan == "itsat"):
             itsat_memory.append(float(value)/1024)
           if(plan == "optic"):
             optic_memory.append(float(value)/1024)

    if(plan == "itsat"):
      itsat_memory.append(hundred_value/1024.0)
    if(plan == "optic"):
      optic_memory.append(hundred_value/1024.0)
        

     
#limitm.append(math.ceil(max([max(mymerge),max(uni),max(naive)])/100)*110.0)      
#limitme.append(math.ceil(max([max(memorys),max(memorym),max(memoryu)])/100)*110.0) 
#limitt.append(math.ceil(max([max(times),max(timem),max(timeu)])/10)*11.0)   

#print limitm, limitme, limitt



#GRAPHS------------------

better=0
worse=0
diff1=[]
diff2=[]




#TIME------------------------------------

fig = pyplot.figure(1)
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

pyplot.axis([0,55, 0.1, 10000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

pyplot.plot(popmer_valid,popmer_time,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{O-VHPOP}$')
pyplot.plot([2,3,4,5],vhpop_time,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-O-VHPOP')
pyplot.plot(popmer_valid,itsat_time,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='ITSAT')
pyplot.plot(popmer_valid,optic_time,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='OPTIC')
pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('tms/time.pdf',bbox_inches='tight',  format = 'pdf')



#MEMORY------------------------------------

fig = pyplot.figure(2)
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

pyplot.axis([0,55, 10,1000])
#pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
pyplot.grid(b=True,which='major')

print vhpop_memory
pyplot.plot(popmer_valid,popmer_memory,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{O-VHPOP}$')
pyplot.plot([2,3,4,5],vhpop_memory,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-O-VHPOP')
pyplot.plot(popmer_valid,itsat_memory,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='*', label='ITSAT')
pyplot.plot(popmer_valid,optic_memory,color='#3d697a', linestyle='None', markeredgecolor='#3d697a', marker='v', label='OPTIC')
pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

pyplot.savefig('tms/memory.pdf',bbox_inches='tight',  format = 'pdf')






