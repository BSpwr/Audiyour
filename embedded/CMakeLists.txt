# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(audiyour)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-but-set-variable" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-parameter" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-variable" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-error=switch" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-switch" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-error=maybe-uninitialized" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-maybe-uninitialized" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-function" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-fpermissive" APPEND)

# littlefs_create_partition_image(littlefs main)