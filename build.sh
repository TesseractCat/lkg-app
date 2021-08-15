cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
# cmake -DCMAKE_BUILD_TYPE=Debug ..
make
mv lkg_app ../lkg_app
cd ..
