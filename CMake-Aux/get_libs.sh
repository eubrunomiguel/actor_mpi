#!/bin/bash

REQUESTED_OPTION=$1

function get_lib_paths {
    UPCXX_LIBS_RAW=$(upcxx-meta LIBFLAGS)
    ACTUAL_LIBS=""

    for lib in $UPCXX_LIBS_RAW
    do
        if [[ $lib =~ ^-L ]]
        then
            ACTUAL_LIBS="$ACTUAL_LIBS;${lib:2}"
        fi
    done

    echo $ACTUAL_LIBS
}

function get_libs {
    UPCXX_LIBS_RAW=$(upcxx-meta LIBFLAGS)
    ACTUAL_LIBS=""

    for lib in $UPCXX_LIBS_RAW
    do
        if [[ $lib =~ ^-l ]]
        then
            ACTUAL_LIBS="$ACTUAL_LIBS;${lib:2}"
        fi
    done

    echo $ACTUAL_LIBS
}

function get_defines {
    UPCXX_LIBS_RAW=$(upcxx-meta PPFLAGS)
    ACTUAL_LIBS=""

    for lib in $UPCXX_LIBS_RAW
    do
        if [[ $lib =~ ^-D ]]
        then
            ACTUAL_LIBS="$ACTUAL_LIBS;$lib"
        fi
    done

    echo $ACTUAL_LIBS
}

function get_includes {
    UPCXX_LIBS_RAW=$(upcxx-meta PPFLAGS)
    ACTUAL_LIBS=""

    for lib in $UPCXX_LIBS_RAW
    do
        if [[ $lib =~ ^-I ]]
        then
            ACTUAL_LIBS="$ACTUAL_LIBS;${lib:2}"
        fi
    done

    echo $ACTUAL_LIBS
}

if [ "$REQUESTED_OPTION" == "libs" ]; then 
    get_libs
elif [ "$REQUESTED_OPTION" == "libpaths" ]; then
    get_lib_paths
elif [ "$REQUESTED_OPTION" == "defines" ]; then
    get_defines
elif [ "$REQUESTED_OPTION" == "includes" ]; then
    get_includes
else
    echo "ERROR!"
fi
