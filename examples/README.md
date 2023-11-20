# roboflex.transport.zmq examples


## 1. **pub_sub_0** 

Tests sending TensorMessage (which uses xtensor or numpy, in the case of python) over ZMQ. Should look exactly the same as core/examples/tensor_0.cpp, but with a ZMQ pub-sub step.

c++: [pub_sub_0_cpp.cpp](pub_sub_0_cpp.cpp)                
python: [pub_sub_0_py.py](pub_sub_0_py.py)

cmake -DCMAKE_INSTALL_PREFIX=/tmp/roboflex_output_tst_2 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wcast-qual" ..