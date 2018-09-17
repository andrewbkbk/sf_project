1) To build the project:

cd source_root
mkdir build
cd build
cmake ..
make

2) To run dynamic-loader (from binary dir: source_root/build):

//pwd -> source_root/build

./dynamic-loader plugins

The loader will try to open every library ignoring incompatible files.
The plugin's name, which it will run, is libbandwidth_prober.so

IMPORTANT!: A class which manages plugins has an option to check plugin's
            API version and API name. By default, dynamic-loader disables
	    this option, but test_loader enables it (there is a test on API
	    compatibility). A plugin with the wrong version or name is
	    silently ignored.

3) To run tests

cd tests
//pwd -> source_root/build/tests
../../tests/run_tests.sh

The script will run test_loader binary with different test plugins.
The are 19 test plugins libplugin_* in the test directory, which
are used for testing.
