import matplotlib
matplotlib.use('SVG')

import numpy as np
import math
import matplotlib.pyplot as pyplot
from array import array
from matplotlib.backends.backend_pdf import PdfPages


problems = ["dae_30min_6GB_10", "dae_30min_6GB_128", "dae_30min_6GB_1024", "dae_30min_6GB_2014", "dae_30min_6GB_4067", "itsat_30min_6GB", "tfd_30min_6GB", "yahsp_30min_6GB"]

for what in problems:
 iterator=1
 last_order=1
 time_str=""
 memory_str=""
 output_time = open('/data/Phd_final/Phd_evaluation/durative/results/driverlog/'+what+'/sep_time', 'w')
 output_memory = open('/data/Phd_final/Phd_evaluation/durative/results/driverlog/'+what+'/sep_memory', 'w')

 with open('/data/Phd_final/Phd_evaluation/durative/results/driverlog/'+what+'/conti','r') as filei:
  print what
  for line in filei:
     data = line.split(";")

     if(data[0]==""):
       continue
     #print data
     fi=float(data[1])
     order=int(data[2])
     time=data[3]
     memory=data[4].split(" ")[0]
     
     if(iterator != fi): #it is not same file
        output_time.write(time_str)
        output_time.write("\n")
        output_memory.write(memory_str)
        output_memory.write("\n")

        time_str = ""
        memory_str = ""
        last_order = 1
        iterator = fi

     if(iterator == fi): #keep making a string
       order=order-1
       if(order > 0):
          if(order == last_order): #no index was skipped
             time_str = time_str + time + ";"
             memory_str = memory_str + memory + ";"
          else: 
             for j in range(last_order, order):
                time_str = time_str +  "-1;"
                memory_str = memory_str + "-1;"
             time_str = time_str + time + ";"
             memory_str = memory_str + memory + ";"

          last_order = order+1
     
 output_time.write(time_str)
 output_memory.write(memory_str)
 output_time.close()
 output_memory.close()
       


