rm -rf build
rm compile_commands.json

mkdir build
cd build
cmake .. -DCMAKE_BUILD_DIR=Release
cmake --build . --clean-first
cd ..
