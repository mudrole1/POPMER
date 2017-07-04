# POPMER
This is a repository for Partially Ordered Plan MERging algorithm POPMER. You can refer to ECAI 2016 paper: [Partial Order Temporal Plan Merging for Mobile Robot Tasks](http://www.cs.bham.ac.uk/~nah/bibtex/papers/mudrova_ecai16.pdf) or contact me for a draft of my PhD thesis.

# Contact
Do not hesitate to contact me with any question: research(the usual symbol for email address)lenka-robotics.eu

# Disclaimer
This code can be compiled and run. However, the code is a result of a developing process, hence it lacks comments for non-experts and it is not well structured. Currently, this code is being improved. Hence, you can contact me and Id provide you with up to date information.

# How to compile it
Even though POPMER can be used with any temporal planner supporting PDDL 2.1., it requires VHPOP. This is due to historic reasons and it is going to be changed in the incoming version. Therefore, you need to get VHPOP. To set it up correctly do:
* git clone https://github.com/mudrole1/POPMER.git
* cd POPMER
* git clone https://github.com/hlsyounes/vhpop.git

The project is set up to used with Qt Creator (https://www.qt.io/ide/) (see Merging.pro file). I don't provide the make file yet. It uses C++11 and it compiles under Ubuntu 14.04 64b without issues. Please contact me, if you experience difficulty. 

# How to run it

