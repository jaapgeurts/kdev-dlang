#!/bin/bash

export XDG_DATA_DIRS=$XDG_DATA_DIRS:$HOME/devinstall/share
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/devinstall/lib64
export QT_PLUGIN_PATH=~/devinstall/lib64/plugins
#export QT_LOGGING_RULES="kdevelop.plugins.kdevd*=true,*language*=true"
export QT_LOGGING_RULES="kdevelop*=true"
kdevelop -s parsertest
