import matplotlib
matplotlib.use('SVG')

import numpy as np
import matplotlib.pyplot as pyplot
from array import array
from matplotlib.backends.backend_pdf import PdfPages


x1 = []
y1 = []
x2 = []
y2 = []

tests = ['bigdeadlines','smaller_deadlines','impossible_deadlines', 'middle_deadlines', 'rand_rel_big_dead','seq_rel_big_dead']
tfiles = ['optic_t', 'optic_t', 'real_runtime','optic_t', 'optic_t','optic_t','optic_t']
miny = [0,-100]
maxy = [1000,0]
IPCsc = []
IPC25s = []
IPC75s = []
solvedM = []
solvedO = []
solvedIntM = []
solvedIntO = []
averageTP = []
p25TP = []
p75TP = []
averageMP = []
p25MP = []
p75MP = []

averageTO = []
p25TO = []
p75TO = []
averageMO = []
p25MO = []
p75MO = []

Pt_big = []
Pt_small = []
Pt_im = []
Pt_rand = []
Pt_seq = []
i = -1
for what in tests:

  POPMER_makespan = []
  OPTIC_makespan = []
  POPMER_time =[]
  OPTIC_time =[]
  POPMER_memory = []
  OPTIC_memory = []
  POPMER_success = []
  OPTIC_success = []

  i = i+1
  print what
  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/POPMER_30min_8GB/mymerge_makespan','r') as file_mymerge:
    for line in file_mymerge:
      line = line.split(";")
      value = line[0]
      if(value == "\n"): #it didnt finish correctly
        POPMER_makespan.append(-100)
        POPMER_success.append(False)
      elif(value == "999999999"): #solution was not found
        POPMER_makespan.append(-1)
        POPMER_success.append(True)
      else: 
        POPMER_makespan.append(float(value))
        POPMER_success.append(True)

  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/pddl3_30min_8GB/optic_makespan','r') as filei:
    for line in filei:
      line = line.split(";")#always x+1
      if(len(line)> 1):
        value = line[len(line)-2]
      else:
        value="\n"

      if(value == "\n"): #it didnt finish correctly
        OPTIC_makespan.append(-100)
        OPTIC_success.append(False)
      elif(value == "999999999"): #solution was not found
        OPTIC_makespan.append(-1)
        OPTIC_success.append(True)
      else: 
        OPTIC_makespan.append(float(value)) 
        OPTIC_success.append(True)

  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/POPMER_30min_8GB/mymerge_t','r') as file_mymerge:
    for line in file_mymerge:
      line = line.split(";")
      m = float(line[0])#[float(i) for i in line.split()]
      if (m != -1):
        POPMER_time.append(m)
      else:
        POPMER_time.append(0)

  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/pddl3_30min_8GB/'+tfiles[i],'r') as filei:
    for line in filei:
      if(tfiles[i] == 'optic_t'):
        line = line.split(";")
        if(len(line)> 1):
          m = float(line[len(line)-2])
        else:
          m=1800
      else:
        if((line[0]=='') or (line[0]==' ')):
          continue
        m=float(line)
      if (m != -1):
        OPTIC_time.append(m)
      else:
        OPTIC_time.append(0) 

  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/POPMER_30min_8GB/mymerge_vm','r') as file_mymerge:
    for line in file_mymerge:
      line = line.split(";")
      m = float(line[0])#[float(i) for i in line.split()]
      if (m != -1):
        POPMER_memory.append(m/1024.0)
      else:
        POPMER_memory.append(0)

  with open('/data/Phd_final/Phd_evaluation/deadlines/results/'+what+'/pddl3_30min_8GB/memory','r') as filei:
    for line in filei:
      line = line.split(" ")
      if((line[0]=='') or (line[0]==' ')):
        continue
      m = float(line[2])#[float(i) for i in line.split()]
      if (m != -1):
        OPTIC_memory.append(m/1024.0)
      else:
        OPTIC_memory.append(0) 
       
  #best makespan/ other makespan
  IPCscore_vector = []
  unsolvedM = 0
  int_unsolvedM = 0
  unsolvedO = 0
  int_unsolvedO = 0

  newM = []
  newO = []
  POPMER_score = []
  x= []
  j=0
  #-1 solution wasnt found intentionally, -100 the algorithm has been killed
  for (m, n) in zip(POPMER_makespan, OPTIC_makespan):
    j = j+1
    if(what=="impossible_deadlines"):
      if( m == -1):
        int_unsolvedM = int_unsolvedM +1
        POPMER_score.append(1)
        x.append(j)
      else:
        unsolvedM = unsolvedM +1
      if(n ==-1):
        if(m == -1):
          IPCscore_vector.append(1)
      elif(n < -1):
        if(m == -1):
          int_unsolvedO = int_unsolvedO +1
          IPCscore_vector.append(0)
          unsolvedO = unsolvedO+1
    else:
      if( m == -100):
        unsolvedM = unsolvedM+1
        continue
      elif(m == -1):
        int_unsolvedM = int_unsolvedM +1
      else:
        POPMER_score.append(1)
        newM.append(m)
        x.append(j)

      if(n == -100):
        unsolvedO = unsolvedO+1
        if(m>0):
          IPCscore_vector.append(0) 
      elif(n == -1):
        int_unsolvedO = int_unsolvedO+1
        if(m>0):
          IPCscore_vector.append(0)
      else:
        IPCscore_vector.append(m/(n-j)) #-j to get away the deliver action effect     
        newO.append(n)

  #print "unsolved", unsolvedM, " ", unsolvedO
  #print "intunolved", int_unsolvedM, " ", int_unsolvedO
  #print "aipcscore", (sum(IPCscore_vector))
  if(len(IPCscore_vector) > 0):
    IPCsc.append(round(np.percentile(IPCscore_vector, 50)*100 - 100))
    IPC25s.append(round(np.percentile(IPCscore_vector, 25)*100 - 100))
    IPC75s.append(round(np.percentile(IPCscore_vector, 75)*100 - 100))
  else:
    IPCsc.append(-1)
    IPC25s.append(-1)
    IPC75s.append(-1)

  solvedM.append(len(POPMER_makespan)-unsolvedM)
  solvedO.append(len(OPTIC_makespan)-unsolvedO)

  solvedIntM.append(len(POPMER_makespan)-int_unsolvedM)
  solvedIntO.append(len(POPMER_makespan)-int_unsolvedO)
  
  averageTP.append(round(np.percentile(POPMER_time, 50),2))
  p25TP.append(round(np.percentile(POPMER_time, 25),2))
  p75TP.append(round(np.percentile(POPMER_time, 75),2))

  averageMP.append(round(np.percentile(POPMER_memory, 50),2))
  p25MP.append(round(np.percentile(POPMER_memory, 25),2))
  p75MP.append(round(np.percentile(POPMER_memory, 75),2))

  averageTO.append(round(np.percentile(OPTIC_time, 50),2))
  p25TO.append(round(np.percentile(OPTIC_time, 25),2))
  p75TO.append(round(np.percentile(OPTIC_time, 75),2))

  averageMO.append(round(np.percentile(OPTIC_memory, 50),2))
  p25MO.append(round(np.percentile(OPTIC_memory, 25),2))
  p75MO.append(round(np.percentile(OPTIC_memory, 75),2))

  pyplot.rc('text', usetex=True)
  pyplot.rc('font', family='serif', size='20')
  pyplot.rc('legend', fontsize= '15', numpoints='1')

  ##problems vs makespan
  fig = pyplot.figure(3*i+1)
  
  ax = fig.add_subplot(111)

  #if(what == 'seq_rel_big_dead'):
  #  ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  #to make distance between description and the numbers bigger
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.xlabel('number of goals literals')
  #pyplot.ylabel('makespan [min]')
  pyplot.ylabel('$\mathit{RIPCscore}$')

  if(what == 'bigdeadlines'):
    pyplot.axis([0,25,-0.1, 1.1])
  if(what == 'smaller_deadlines'):
    pyplot.axis([0,25,-0.1, 1.1])
  elif(what == 'impossible_deadlines'):
    pyplot.axis([0,25,-0.1,1.1])
  elif(what == 'rand_rel_big_dead'):
    pyplot.axis([0,25,-0.1,1.1])
  else:
    pyplot.axis([0,25,-0.1,1.1])
     
  pyplot.grid(b=True,which='major')

  print len(x), len(IPCscore_vector)
  pyplot.plot(x,POPMER_score,color='#1b2c3f', linestyle='None', marker='o',markeredgecolor='#1b2c3f',label='POPMER$_{\mathit{IPC3-VHPOP}}$')
  pyplot.plot(x,IPCscore_vector,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='v', label='OPTIC')

  #pyplot.legend(loc='upper left')
  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, mode="expand", borderaxespad=0.)
  
  pyplot.savefig(what+'/score_'+what+'.pdf',bbox_inches='tight', format = 'pdf')


  ##problems vs time
  fig = pyplot.figure(3*i+2)
  
  ax = fig.add_subplot(111)
  ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  #to make distance between description and the numbers bigger
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.xlabel('number of goals literals')
  pyplot.ylabel('runtime [s]')

  x = range(1,24)
  max_time = [1800]*26
  max_x = range(0,26)

  pyplot.axis([0,25, 0.1,10000])
  pyplot.grid(b=True,which='major')

  pyplot.plot(x,POPMER_time,color='#1b2c3f', linestyle='None', marker='o',markeredgecolor='#1b2c3f',label='POPMER$_{\mathit{IPC3-VHPOP}}$')
  pyplot.plot(x,OPTIC_time,color='#f4651a', linestyle='None', marker='v',markeredgecolor='#f4651a', label='OPTIC')
  pyplot.plot(max_x,max_time,color='red', linestyle='-', label='maximal allowed time')

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, mode="expand", borderaxespad=0.)
  pyplot.savefig(what+'/runtime_'+what+'.pdf',bbox_inches='tight', format = 'pdf')
  

  ##problems vs memory
  fig = pyplot.figure(3*i+3)
  
  ax = fig.add_subplot(111)
  ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  #to make distance between description and the numbers bigger
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.xlabel('number of goals literals')
  pyplot.ylabel('memory [MB]')

  x = range(1,24)
  max_memory = [8*1024]*26
  pyplot.axis([0,25, 10,10000])
  pyplot.grid(b=True,which='major')

  pyplot.plot(x,POPMER_memory,color='#1b2c3f', linestyle='None', marker='o',markeredgecolor='#1b2c3f',label='POPMER$_{\mathit{IPC3-VHPOP}}$')
  pyplot.plot(x,OPTIC_memory,color='#f4651a', linestyle='None', marker='v',markeredgecolor='#f4651a', label='OPTIC')
  pyplot.plot(max_x,max_memory,color='red', linestyle='-', label='maximal allowed memory')

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, mode="expand", borderaxespad=0.)
  pyplot.savefig(what+'/memory_'+what+'.pdf',bbox_inches='tight', format = 'pdf')

  if(what == 'bigdeadlines'):
    Pt_big = POPMER_time
  elif(what == 'smaller_deadlines'):
    Pt_small = POPMER_time
  elif(what == 'impossible_deadlines'):
    Pt_im = POPMER_time
  elif(what == 'rand_rel_big_dead'):
    Pt_rand = POPMER_time
  elif(what == 'seq_rel_big_dead'):
    Pt_seq = POPMER_time


#save to file the score
target = open("scores", 'w')
target.write("ipc-score median & ipc-score 25s & ipc-score 75s & solvedPOPMER & solvedOPTIc & \n")
for i in range(0,len(IPCsc)):
  target.write(tests[i] + " & " + str(IPCsc[i]) + " & " + str(IPC25s[i]) + " & " + str(IPC75s[i]) + " & " + str(solvedM[i]) + " & " +str(solvedO[i]) + " \\\\ \n")
target.close()

target = open("times", 'w')
target.write("medianTimeP &  25stimeP &  75stimeP &  medianTimeO &  25stimeO &  75stimeO & \n")
for i in range(0,len(IPCsc)):
  target.write(tests[i] + " & " + str(averageTP[i]) + " & " +str(p25TP[i]) + " & " + str(p75TP[i]) + " & ")
  target.write(str(averageTO[i]) + " & " +str(p25TO[i]) + " & " + str(p75TO[i]) + " \\\\ \n") 
target.close()

target = open("memory", 'w')
target.write("medianMemP &  25smMemP &  75sMemP &  medianMemO &  25smMemO &  75sMemO & \n")
for i in range(0,len(IPCsc)):
  target.write(tests[i] + " & " + str(averageMP[i]) + " & " +str(p25MP[i]) + " & " + str(p75MP[i]) + " & ")
  target.write(str(averageMO[i]) + " & " +str(p25MO[i]) + " & " + str(p75MO[i]) + " \\\\ \n") 
target.close()

##problems vs memory
fig = pyplot.figure(3*i+4)
  
ax = fig.add_subplot(111)
ax.set_yscale('log')
ax.tick_params(axis='both', which='major', labelsize=20)
#to make distance between description and the numbers bigger
ax.set_ylabel('ylabel',labelpad=5)
ax.set_xlabel('xlabel',labelpad=10)
pyplot.xlabel('number of goals literals')
pyplot.ylabel('runtime [s]')

x = range(1,24)
max_memory = [8*1024]*26
pyplot.axis([0,25, 0.1,1000])
pyplot.grid(b=True,which='major')

pyplot.plot(x,Pt_im,color='#a68900', linestyle='None', marker='D',markeredgecolor='#a68900', label='ITW')
pyplot.plot(x,Pt_big,color='#1b2c3f', linestyle='None', marker='o',markeredgecolor='#1b2c3f',label='LTW')
pyplot.plot(x,Pt_small,color='#f4651a', linestyle='None', marker='v',markeredgecolor='#f4651a', label='STW')
pyplot.plot(x,Pt_seq,color='#fca771', linestyle='None', marker='s',markeredgecolor='#fca771', label='SR')
pyplot.plot(x,Pt_rand,color='#3d697a', linestyle='None', marker='*',markeredgecolor='#3d697a', label='RR')


pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, mode="expand", borderaxespad=0.)
pyplot.savefig('all_popmer.pdf',bbox_inches='tight', format = 'pdf')







