
find_package(GTest)
if (GTest_FOUND)
    project(${PROJECT_NAME}.test)

    include_directories(../ctsm)
    add_executable(${PROJECT_NAME} main.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ctsm GTest::GTest)
endif ()