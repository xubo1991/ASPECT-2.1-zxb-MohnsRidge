#!/bin/bash
#
# This script compiles all plugins and runs all prm files in the subdirectories
# of the cookbooks folder

if [ "$#" -ne 1 ]; then
    echo "usage: $0 aspect-build-directory"
    exit 1
fi

BUILD=`cd $1;pwd`

if [[ ! -e $BUILD/AspectConfig.cmake || ! -e $BUILD/aspect  ]];
then
    echo "'$BUILD' doesn't look like a build directory"
    exit 1
fi


# run aspect on all .prm files in the current folder
run_all_prms ()
{
    for prm in `ls *prm`;
    do
    if [ "`basename $prm`" = "parameters.prm" ];
    then
	continue;
    fi
    echo "Running '$prm' at `pwd` with '$BUILD' ..."
    cp $prm $prm.tmp
    echo "set End time=0" >> $prm.tmp
    echo "set Max nonlinear iterations = 5" >> $prm.tmp

    $BUILD/aspect $prm.tmp >/dev/null || { rm -f $prm.tmp; return 2; }
    rm -f $prm.tmp
    done
    return 0;
}

# configure and compile the plugin in the current directory
make_lib ()
{
    echo "configuring in `pwd` ..."
    rm -rf CMakeCache.txt
    cmake -D Aspect_DIR=$BUILD . >/dev/null || { echo "cmake failed!"; return 1; }
    make >/dev/null || { echo "make failed!"; return 2; }
    return 0;
}


echo "Checking cookbooks using $BUILD/aspect ..."

( (run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd finite_strain; make_lib && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd future && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd inner_core_convection; make_lib && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd prescribed_velocity; make_lib && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd free-surface-with-crust/plugin && make_lib && cd .. && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd sinker-with-averaging; run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd morency_doin_2004; make_lib && run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd geomio; run_all_prms ) || { echo "FAILED"; exit 1; } ) &

( (cd muparser_temperature_example; run_all_prms ) || { echo "FAILED"; exit 1; } ) &



wait

echo "all good! :-)"
exit 0
