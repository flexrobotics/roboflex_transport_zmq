cmake_minimum_required(VERSION 3.24)

project(roboflex_transport_zmq VERSION 0.1.5 DESCRIPTION "roboflex transport zmq")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# -------------------- 
# Resolve dependencies

# Include FetchContent Module
include(FetchContent)

# download and build roboflex_core
FetchContent_Declare(roboflex_core
    GIT_REPOSITORY https://github.com/flexrobotics/roboflex.git
    GIT_TAG        main
)
set(BUILD_ROBOFLEX_PYTHON_EXT OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(roboflex_core)

find_package(ZeroMQ QUIET)
if(NOT ZeroMQ_FOUND)
    message(">>>>>>>>>> LIBZMQ NOT FOUND; WE WILL BUILD IT OURSELVES <<<<<<<<<<<<")

    # Fetch and make libzmq available.
    # YES WE'RE GONNA COMPILE IT OURSELVES!!!
    FetchContent_Declare(
        libzmq
        GIT_REPOSITORY https://github.com/zeromq/libzmq.git
        GIT_TAG        v4.3.5
    )
    FetchContent_MakeAvailable(libzmq)
else()
    message(">>>>>>>>>>>> LIBZMQ FOUND THROUGH find_package(ZeroMQ QUIET) <<<<<<<<<<<<<<<<")
endif()

FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG        v4.10.0
)
add_library(cppzmq INTERFACE)
target_include_directories(cppzmq INTERFACE 
    $<BUILD_INTERFACE:${cppzmq_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:include>
)


# -------------------- 
# Define the library

add_library(roboflex_transport_zmq STATIC
    src/zmq_nodes.cpp
    include/roboflex_transport_zmq/zmq_nodes.h
)

# Set some properties on our library
set_property(TARGET roboflex_transport_zmq PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)

# Include directories when we compile our library
target_include_directories(roboflex_transport_zmq PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
    ${ZeroMQ_INCLUDE_DIRS}
)

# Link against the necessary libraries
target_link_libraries(roboflex_transport_zmq PUBLIC 
    roboflex_core 
    libzmq-static
    cppzmq

    # Unfortunately, we need to link against all these libraries
    # because they are used by roboflex_core, and cmake doesn't
    # seem to be transitivily doing it's thing. Urgh, someone help.
    flatbuffers_util 
    xtensor 
    xsimd 
    xtl
    Eigen3::Eigen
)


# -------------------- 
# Examples

# pub_sub_0_cpp example
add_executable(pub_sub_0_cpp examples/pub_sub_0_cpp.cpp)
target_link_libraries(pub_sub_0_cpp PRIVATE 
    roboflex_core 
    roboflex_transport_zmq
)


# -------------------- 
# install

# If you need to install the transport_zmq library
install(TARGETS roboflex_transport_zmq 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
#install(FILES transport_zmq.h DESTINATION include/roboflex_transport_zmq)
install(DIRECTORY include/roboflex_transport_zmq
    DESTINATION include
)

install(TARGETS roboflex_transport_zmq
    EXPORT roboflex_transport_zmqTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(TARGETS cppzmq
    EXPORT roboflex_transport_zmqTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# export the targets to a make script
install(EXPORT roboflex_transport_zmqTargets
    FILE roboflex_transport_zmqTargets.cmake
    #NAMESPACE roboflex_core:: # do we want this?
    DESTINATION lib/cmake/roboflex_transport_zmq # do we want or need cmake/ ?
)


# --------------------
# build python bindings

add_subdirectory(python)
