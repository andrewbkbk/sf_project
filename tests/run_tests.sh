#!/bin/bash

TEST_LOADER="./test_loader"

if [ ! -x $TEST_LOADER ]; then
    echo "error, incorrect current directory"
    echo "test binary $TEST_LOADER must be in current directory"
    exit 1
fi

PLUGINS=`ls libplugin_*.so|wc -l`

if [ $PLUGINS -ne 19 ]; then
    echo "error, incorrect current directory"
    echo "expected to find 19 plugin libraries libplugin_*.so, found $PLUGINS"
    exit 1
fi

# this directory is hardcoded in TEST_LOADER as a plugin directory
PLUGIN_DIR="temp_test_plugins"

# clean up plugin dir
mkdir -p $PLUGIN_DIR
rm -f $PLUGIN_DIR/*

# copy plugins
cp libplugin_* $PLUGIN_DIR/

function get_num_unique_threads()
{
    UNIQUE_THREADS=`grep "Status:" result.txt|grep -v NOT_ON_THREAD|awk '{print $7}'|sort -u|wc -l`
}

# will run in the background
function add_plugins_slowly()
{
    rm -f $PLUGIN_DIR/*
    mkdir -p "temp_test_plugins_TMP"
    rm -f temp_test_plugins_TMP/*
    cp libplugin_* temp_test_plugins_TMP/
    for F in temp_test_plugins_TMP/libplugin_*
    do
       sleep 1
       mv $F $PLUGIN_DIR/
    done
}

function add_remove_add_plugin_slowly
{
    rm -f $PLUGIN_DIR/*
    mkdir -p "temp_test_plugins_TMP"
    rm -f temp_test_plugins_TMP/*
    cp libplugin_1000ms_true.so temp_test_plugins_TMP/
    mv temp_test_plugins_TMP/libplugin_1000ms_true.so $PLUGIN_DIR/
    sleep 5
    rm -f $PLUGIN_DIR/libplugin_1000ms_true.so
    sleep 5
    cp libplugin_1000ms_true.so temp_test_plugins_TMP/
    mv temp_test_plugins_TMP/libplugin_1000ms_true.so $PLUGIN_DIR/
}

echo
echo "#######################################"
echo "running all plugins test, max threads 1"
echo "#######################################"

$TEST_LOADER 46 1 100 | tee result.txt

COUNT_SUCCESS=`grep SUCCESS result.txt | wc -l`

if [ $COUNT_SUCCESS -ne 12 ]; then
    echo "Error, test 01 failed, max threads 1 (48 1 100)"
    echo "COUNT_SUCCESS = $COUNT_SUCCESS, not equal 12"
    exit 1
fi

COUNT_ERRORS=`grep ERROR result.txt | wc -l`

if [ $COUNT_ERRORS -ne 6 ]; then
    echo "Error, test 01 failed, max threads 1 (48 1 100)"
    echo "COUNT_ERRORS = $COUNT_ERRORS, not equal 6"
    exit 1
fi

get_num_unique_threads

if [ $UNIQUE_THREADS -ne 1 ]; then
    echo "Error, test 01 failed, max threads 1 (48 1 100)"
    echo "UNIQUE_THREADS = $UNIQUE_THREADS, not equal 1"
    exit 1
fi

echo "all plugins test, max threads 1 PASSED"

rm -f result.txt

echo
echo "#######################################"
echo "running all plugins test, max threads 2"
echo "#######################################"

$TEST_LOADER 35 2 100 | tee result.txt

COUNT_SUCCESS=`grep SUCCESS result.txt | wc -l`

if [ $COUNT_SUCCESS -ne 12 ]; then
    echo "Error, test 02 failed, max threads 2 (35 2 100)"
    echo "COUNT_SUCCESS = $COUNT_SUCCESS, not equal 12"
    exit 1
fi

COUNT_ERRORS=`grep ERROR result.txt | wc -l`

if [ $COUNT_ERRORS -ne 6 ]; then
    echo "Error, test 02 failed, max threads 2 (35 2 100)"
    echo "COUNT_ERRORS = $COUNT_ERRORS, not equal 6"
    exit 1
fi

get_num_unique_threads

if [ $UNIQUE_THREADS -ne 2 ]; then
    echo "Error, test 02 failed, max threads 2 (35 2 100)"
    echo "UNIQUE_THREADS = $UNIQUE_THREADS, not equal 2"
    exit 1
fi

echo "all plugins test, max threads 2 PASSED"

rm -f result.txt

echo
echo "#######################################"
echo "running all plugins test, max threads 5"
echo "#######################################"

$TEST_LOADER 16 5 100 | tee result.txt

COUNT_SUCCESS=`grep SUCCESS result.txt | wc -l`

if [ $COUNT_SUCCESS -ne 12 ]; then
    echo "Error, test 03 failed, max threads 5 (16 5 100)"
    echo "COUNT_SUCCESS = $COUNT_SUCCESS, not equal 12"
    exit 1
fi

COUNT_ERRORS=`grep ERROR result.txt | wc -l`

if [ $COUNT_ERRORS -ne 6 ]; then
    echo "Error, test 03 failed, max threads 5 (16 5 100)"
    echo "COUNT_ERRORS = $COUNT_ERRORS, not equal 6"
    exit 1
fi

get_num_unique_threads

if [ $UNIQUE_THREADS -ne 5 ]; then
    echo "Error, test 03 failed, max thread 5 (16 5 100)"
    echo "UNIQUE_THREADS = $UNIQUE_THREADS, not equal 5"
    exit 1
fi

echo "all plugins test, max threads 5 PASSED"
rm -f result.txt

echo
echo "####################################################"
echo "running all plugins added slowly test, max threads 3"
echo "####################################################"

add_plugins_slowly &
$TEST_LOADER 38 3 1 | tee result.txt
wait

COUNT_SUCCESS=`grep SUCCESS result.txt | wc -l`

if [ $COUNT_SUCCESS -ne 12 ]; then
    echo "Error, test 04 failed, max threads 2 (35 2 100)"
    echo "COUNT_SUCCESS = $COUNT_SUCCESS, not equal 12"
    exit 1
fi

COUNT_ERRORS=`grep ERROR result.txt | wc -l`

if [ $COUNT_ERRORS -ne 6 ]; then
    echo "Error, test 04 failed, max threads 2 (35 2 100)"
    echo "COUNT_ERRORS = $COUNT_ERRORS, not equal 6"
    exit 1
fi
echo "all plugins added slowly test, max threads 3, PASSED"

echo
echo "#############################################################"
echo "running add, remove and add slowly plugin test, max threads 2"
echo "#############################################################"

add_remove_add_plugin_slowly &
$TEST_LOADER 18 2 1 | tee result.txt
wait

COUNT_SUCCESS=`grep SUCCESS result.txt | wc -l`

if [ $COUNT_SUCCESS -ne 2 ]; then
    echo "Error, test 05 failed, max threads 2 (15 2 1)"
    echo "COUNT_SUCCESS = $COUNT_SUCCESS, not equal 2"
    exit 1
fi

COUNT_ERRORS=`grep ERROR result.txt | wc -l`

if [ $COUNT_ERRORS -ne 0 ]; then
    echo "Error, test 05 failed, max threads 2 (15 2 1)"
    echo "COUNT_ERRORS = $COUNT_ERRORS, not equal 0"
    exit 1
fi
echo "add, remove and add slowly plugin test, max threads 2, PASSED"
