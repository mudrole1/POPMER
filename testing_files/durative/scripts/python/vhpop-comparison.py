import matplotlib
matplotlib.use('SVG')

import numpy as np
import math
import matplotlib.pyplot as pyplot
from array import array
from matplotlib.backends.backend_pdf import PdfPages

IPC3 = ["old/POPMER_IPCVHPOP_30min_8GB", "old/ipc3_vhpop_sequence_30min_8GB", "old/ipc3_vhpop_full_30min_8GB"]
ORIG = ["POPMER_0VHPOP_30min_6GB", "old/o_vhpop_sequence_30min_8GB", "o_vhpop_full_30min_6GB"]
FIG= ["IPC3-VHPOP", "O-VHPOP"]
limitm = []
limitme = []
limitt = []
limituS=[]
limitlS=[]

limituX = []
limitlX = []

tests = [IPC3, ORIG]

g=-1

for what in tests:
 
  g=g+1
  mymerge = []
  naive = []
  uni = []

  validm = []
  valids = []
  validu = []

  memorym = []
  memorys = []
  memoryu = []

  timem = []
  times = []
  timeu = []

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[0]+ '/makespan','r') as file_mymerge:
    for line in file_mymerge:
     makespans = line.split(";")
     value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
     mymerge.append(float(value))

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[1]+ '/makespan','r') as filei:
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] 
    naive.append(float(value))

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[2]+ '/makespan','r') as filei:
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] 
    uni.append(float(value))

  #MEMORY -------------
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[0]+ '/memory','r') as file_mymerge:
   for line in file_mymerge:
    makespans = line.split(" ")
    if(len(makespans)==4):   
      value=float(makespans[2])/1024.0 
      memorym.append(float(value))

  max_memory = 0
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[1]+ '/memory','r') as filei:
   for line in filei:
    makespans = line.split(" ")
    if(len(makespans)==4):
      value=float(makespans[2])/1024.0  
      if(value > max_memory):
        memorys.append(float(value))
        max_memory = value
      else:
        memorys.append(max_memory)

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[2]+ '/memory','r') as filei:
   for line in filei:
    makespans = line.split(" ")
    if(len(makespans)==4):
      value=float(makespans[2])/1024.0  
      memoryu.append(float(value))


  #TIME ------------
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[0]+ '/real_runtime','r') as file_mymerge:
   i=0
   for line in file_mymerge:
    if(i==0):
      i = i+1
      continue

    makespans = line.split("\n")
    if(len(makespans)==2):
      value=makespans[0]
      timem.append(float(value)) 

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[1]+ '/real_runtime','r') as filei:
   i=0
   for line in filei:
    if(i==0):
      i = i+1
      continue

    makespans = line.split("\n")
    if(len(makespans)==2):
      value=float(makespans[0])
      if(len(times)>0):
        ov = times[len(times)-1]
      else:
        ov =0
      times.append(ov+value)

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[2]+ '/real_runtime','r') as filei:
   i=0
   for line in filei:
    if(i==0):
      i = i+1
      continue

    makespans = line.split("\n")
    if(len(makespans)==2):
      value=makespans[0]
      timeu.append(float(value))


  #VALID----------
  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[0]+ '/valid','r') as file_mymerge:
   x=1
   invalid =0
   for line in file_mymerge:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if(value=="1"):
      validm.append(x)
    else:
      del mymerge[x-1-invalid]
      del memorym[x-1-invalid]
      del timem[x-1-invalid]
      invalid = invalid+1
    x=x+1

  limitlX.append(min(validm)-1)
  limituX.append(max(validm)+1)

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[1]+ '/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if((value=="1") and (x < limituX[len(limituX)-1])):
      valids.append(x)
    else:
      del naive[x-1-invalid]
      del memorys[x-1-invalid]
      del times[x-1-invalid]
      invalid = invalid+1
    x=x+1

  with open('/home/lenka/PostDoc/POPMER/testing_files/durative/results/vhpop_comparison/' + what[2]+ '/valid','r') as filei:
   x=1
   invalid =0
   for line in filei:
    makespans = line.split(";")
    value=makespans[len(makespans)-2] #-1 gives \n, -2 the last number
    if((value=="1") and (x < limituX[len(limituX)-1])):
      validu.append(x)
    else:
      del uni[x-1-invalid]
      del memoryu[x-1-invalid]
      del timeu[x-1-invalid]
      invalid = invalid+1
    x=x+1

  #ICPScore criterion

  #I can compute only for validm indexed

  scoreS = []
  scoreM = []
  lastIndex=0
  for i in range(0,len(validm)): 
    scoreM.append(1) 
    if(lastIndex >= len(valids)):
         scoreS.append(0)
    else:
      if(validm[i] == valids[lastIndex]):
         scoreS.append(float(mymerge[i])/float(naive[lastIndex]))
         lastIndex = lastIndex+1
      elif(validm[i] < valids[lastIndex]): #my merge found sol, but the other not
         scoreS.append(0)
    #else: #my merge hasnt found sol, but the other did

  scoreU = []
  lastIndex=0
  for i in range(0,len(validm)):  
    if(lastIndex >= len(validu)):
         scoreU.append(0)
    else:
      if(validm[i] == validu[lastIndex]):
         scoreU.append(float(mymerge[i])/float(uni[lastIndex]))
         lastIndex = lastIndex+1
      elif(validm[i] < validu[lastIndex]): #my merge found sol, but the other not
         scoreU.append(0)
         
  limitm.append(math.ceil(max([max(mymerge),max(uni),max(naive)])/100)*110.0)      
  limitme.append(math.ceil(max([max(memorys),max(memorym),max(memoryu)])/100)*110.0) 
  limitt.append(math.ceil(max([max(times),max(timem),max(timeu)])/10)*11.0)   

  print limitm, limitme, limitt

  limituS.append(math.ceil(max([max(scoreS),max(scoreU)]))+0.1)
  limitlS.append(math.floor(min([min(scoreS),min(scoreU)])*10)/10.0-0.1)


  #GRAPHS------------------

  fig = pyplot.figure(g*4+1)
  ax = fig.add_subplot(111)
  #ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.rc('text', usetex=True)
  pyplot.rc('font', family='serif', size='20')
  pyplot.rc('legend', fontsize= '15', numpoints='1')
   
  pyplot.xlabel('number of goal literals',fontsize=20)
  pyplot.ylabel('makespan [min]',fontsize=20)

  pyplot.axis([limitlX[g],limituX[g], 0,limitm[g]])
  #pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
  pyplot.grid(b=True,which='major')

  pyplot.plot(valids,naive,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='s', label='S-'+FIG[g])
  pyplot.plot(validu,uni,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-'+FIG[g])
  pyplot.plot(validm,mymerge,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{'+FIG[g]+'}$')

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)



  pyplot.savefig('vhpop_comparison_graphs/g-makespan-'+FIG[g]+'.svg',bbox_inches='tight',  format = 'svg')



  #MEMORY-------------------------

  fig = pyplot.figure(g*4+2)
  ax = fig.add_subplot(111)
  ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.xlabel('number of goal literals',fontsize=20)
  pyplot.ylabel('memory [MB]',fontsize=20)

  pyplot.axis([limitlX[g],limituX[g], 10,limitme[g]])
  #pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
  pyplot.grid(b=True,which='major')

  pyplot.plot(valids,memorys,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='s', label='S-'+FIG[g])
  pyplot.plot(validu,memoryu,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-'+FIG[g])
  pyplot.plot(validm,memorym,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{'+FIG[g]+'}$')

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)  

  pyplot.savefig('vhpop_comparison_graphs/g-memory-'+FIG[g]+'.svg',bbox_inches='tight',  format = 'svg')
 
  #TIME-------------------------

  fig = pyplot.figure(g*4+3)
  ax = fig.add_subplot(111)
  ax.set_yscale('log')
  ax.tick_params(axis='both', which='major', labelsize=20)
  ax.set_ylabel('ylabel',labelpad=5)
  ax.set_xlabel('xlabel',labelpad=10)

  pyplot.xlabel('number of goal literals',fontsize=20)
  pyplot.ylabel('runtime [s]',fontsize=20)

  pyplot.axis([limitlX[g],limituX[g], 0.1,limitt[g]])
  #pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
  pyplot.grid(b=True,which='major')

  pyplot.plot(valids,times,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='s', label='S-'+FIG[g])
  pyplot.plot(validu,timeu,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-'+FIG[g])
  pyplot.plot(validm,timem,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{'+FIG[g]+'}$')

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

  pyplot.savefig('vhpop_comparison_graphs/g-time-'+FIG[g]+'.svg',bbox_inches='tight',  format = 'svg')

  #IPCscore---------
  fig = pyplot.figure(g*4+4)
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

  pyplot.axis([limitlX[g],limituX[g], limitlS[g],limituS[g]])
  #pyplot.xticks([0,6,12,18,24], [0,6,12,18,24])
  pyplot.grid(b=True,which='major')

  pyplot.plot(validm,scoreM,color='#1b2c3f', linestyle='None', markeredgecolor='#1b2c3f', marker='o', label='POPMER$_{'+FIG[g]+'}$')
  pyplot.plot(validm,scoreS,color='#f4651a', linestyle='None', markeredgecolor='#f4651a', marker='s', label='S-'+FIG[g])
  pyplot.plot(validm,scoreU,color='#a68900', linestyle='None', markeredgecolor='#a68900', marker='D', label='C-'+FIG[g])
  

  pyplot.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,  ncol=2, mode="expand", borderaxespad=0.)

  pyplot.savefig('vhpop_comparison_graphs/g-RIPCscore-'+FIG[g]+'.svg',bbox_inches='tight',  format = 'svg')



