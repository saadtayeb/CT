cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(pfe VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_BUILD_TYPE Release)


include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM gtkmm-3.0) 
find_package( OpenCV REQUIRED )
set(Torch_DIR libtorch)
find_package(Torch PATHS ${Torch_DIR} REQUIRED)


add_executable(pfe
cpp/main.cpp
cpp/myarea.cpp
cpp/commands.cpp
cpp/deep-learning.cpp
)



target_include_directories(pfe PUBLIC hpp)
target_include_directories(pfe PUBLIC cpp)
include_directories(${OpenCV_INCLUDE_DIRS}
			    ${TORCH_INCLUDE_DIRS}
			    ${GTKMM_INCLUDE_DIRS})

target_link_libraries(pfe ${OpenCV_LIBS}
		          ${TORCH_LIBRARIES} 
		          ${GTKMM_LIBRARIES})
		          
set_property(TARGET pfe PROPERTY CXX_STANDARD 14)

