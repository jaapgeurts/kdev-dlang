#!/bin/bash

export XDG_DATA_DIRS=$XDG_DATA_DIRS:$HOME/devinstall/share
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/devinstall/lib64
export QT_LOGGING_RULES="kdevelop.plugins.*=true"
export QT_PLUGIN_PATH=~/devinstall/lib64/plugins
unset DEBUGINFOD_URLS
export DRT_GCOPT="gcopt=parallel:1 profile:1"

#valgrind --tool=memcheck /usr/bin/kdevelop
gdb --args ~/src/oss/kdevelop/build/bin/kdevelop -s parsertest 
#gdb --args ~/src/oss/kdevelop/build/bin/kdevelop -s mqtt_plasmoid

