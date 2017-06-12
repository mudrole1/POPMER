#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T18:30:00
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Merging
CONFIG   += console
CONFIG   -= app_bundle
CONFIG -= qt
CONFIG += c++11

TEMPLATE = app


SOURCES += main.cpp \
    vhpop/actions.cc \
    vhpop/bindings.cc \
    vhpop/domains.cc \
    vhpop/effects.cc \
    vhpop/expressions.cc \
    vhpop/flaws.cc \
    vhpop/formulas.cc \
    vhpop/functions.cc \
    vhpop/heuristics.cc \
    vhpop/link.cpp \
    vhpop/orderings.cc \
    vhpop/parameters.cc \
    vhpop/pddl.cc \
    vhpop/plans.cc \
    vhpop/predicates.cc \
    vhpop/problems.cc \
    vhpop/refcount.cc \
    vhpop/requirements.cc \
    vhpop/terms.cc \
    vhpop/tokens.cc \
    vhpop/types.cc \
    vhpop/vhpop_callable.cc \
    finalplan.cpp \
    steptimed.cc \
    task.cc \
    vhpop/getopt.c \
    vhpop/getopt1.c \
    myactionschema.cpp \
    parsepddlfiles.cpp \
    mygroundaction.cpp \
    myplan.cpp \
    mylink.cpp

HEADERS += \
    vhpop/actions.h \
    vhpop/bindings.h \
    vhpop/chain.h \
    vhpop/config.h \
    vhpop/debug.h \
    vhpop/domains.h \
    vhpop/effects.h \
    vhpop/expressions.h \
    vhpop/flaws.h \
    vhpop/formulas.h \
    vhpop/functions.h \
    vhpop/getopt.h \
    vhpop/heuristics.h \
    vhpop/link.h \
    vhpop/orderings.h \
    vhpop/parameters.h \
    vhpop/pddl.h \
    vhpop/plans.h \
    vhpop/predicates.h \
    vhpop/problems.h \
    vhpop/refcount.h \
    vhpop/requirements.h \
    vhpop/terms.h \
    vhpop/types.h \
    vhpop/vhpop_callable.h \
    finalplan.h \
    merging.h \
    steptimed.h \
    task.h \
    myactionschema.h \
    parsepddlfiles.h \
    mygroundaction.h \
    myplan.h \
    mylink.h
