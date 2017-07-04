# POPMER
This is a repository for Partially Ordered Plan MERging algorithm POPMER. You can refer to ECAI 2016 paper: [Partial Order Temporal Plan Merging for Mobile Robot Tasks](http://www.cs.bham.ac.uk/~nah/bibtex/papers/mudrova_ecai16.pdf) or contact me for a draft of my PhD thesis.

# Contact
Do not hesitate to contact me with any question: research(the usual symbol for an email address)lenka-robotics.eu

# Disclaimer
This code can be compiled and run. However, the code is a result of a developing process. Hence, it lacks comments for non-experts and it is not well structured. Currently, this code is being improved. Hence, you can contact me and I will provide you with up to date information.

# How to compile it
Even though POPMER can be used with any temporal planner supporting PDDL 2.1., it requires VHPOP. This is due to historic reasons and it is going to be changed in the incoming version. Therefore, you need to get VHPOP. To set it up correctly do:
* `git clone https://github.com/mudrole1/POPMER.git`
* `cd POPMER`
* `git clone https://github.com/hlsyounes/vhpop.git`

The project is set up to used with Qt Creator (https://www.qt.io/ide/) (see Merging.pro file). I don't provide the make file yet. It uses C++11 and it compiles under Ubuntu 14.04 64b without issues. Please contact me, if you experience difficulties to compile. 

# How to run it

## Testing files

I provide several testing files used to run POPMER and evaluate it with other planners. 
 * [Durative domain](testing_files/durative/) includes files to evaluate POPMER and other planners on PDDL 2.1 problems (Driverlog and TMS). You can also find results for different planners and scripts used to visualise these results.
 * [Deadlines domain](testing_files/deadlines/) includes files to evaluate POPMER and OPTIC on PDDL 3.1 problems (Driverlog only). Similarly as before, you can access results and evaluation scripts.
 
 The config files for POPMER has following syntax:
  * First line is reserved for any parameter which needs to be passed to the embedded planner (again, historic reasons, it is going to be changed to make configs planner independent)
  * other lines: release_date;deadline;absolute_path_to_domain;absolute_path_to_problem;\n
  * the last line has NO new line character in the end!

## Running just on a single file

`./Merging file vhpop`

where file is a path to a config file with a problem, for example "durative/domains/driverlog/POPMER/config_01" and vhpop is a path to a binary of a planner you want to use. Notice, it must support format for VAL software. For example, "./Phd/temp_evaluation/vhpop-run/vhpop/vhpop". Notice the "./" at the start of the string!

## Runnning on multiple files

You can use also the [bash script](testing_files/general_scripts/bash/POMer.bash) in order to run on more config files. Set up FILES, DOMAIN and RESULT paths for your needs. 

