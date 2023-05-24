cmake_minimum_required(VERSION 3.14)
project(Blockchain)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g" )
# -g -fsanitize=address -Werror  -DNDEBUG -lboost_system -O3

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)


include_directories(include)
link_directories(include)
add_executable(
  Demo
  src/main.cpp
)

# FetchContent_Declare(json
#   GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
#   GIT_TAG v3.10.4
# )

# FetchContent_GetProperties(json)
# if(NOT json_POPULATED)
#   FetchContent_Populate(json)
#   add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

# target_link_libraries(Demo
#   PRIVATE
#   nlohmann_json::nlohmann_json 
# )

# include_directories(src)
# link_directories(src)


# include(GoogleTest)
# gtest_discover_tests(RTree)
# gtest_discover_tests(RTreeNode)
# gtest_discover_tests(Point)
# gtest_discover_tests(BBox)

# add_custom_command(TARGET Demo PRE_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory
#                        ${CMAKE_SOURCE_DIR}/config/ $<TARGET_FILE_DIR:Demo>)