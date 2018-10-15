cmake_minimum_required(VERSION 3.8)

if ($ENV{TRAVIS_BUILD_NUMBER})
    project(kkmeans VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else ()
    project(kkmeans VERSION 0.0.1)
endif ()

set(CMAKE_CXX_STANDARD 17)

add_executable(kkmeans src/main.cpp)

target_link_libraries(kkmeans
        dlib
        blas
        lapack
        )

add_executable(kkmeans_data_generator src/kkmeans_data_generator.cpp)
target_link_libraries(kkmeans_data_generator
        dlib
        blas
        lapack
        )


add_executable(stars_data_generator src/stars_data_generator.cpp)
target_link_libraries(stars_data_generator
        dlib
        blas
        lapack
        )

install(TARGETS kkmeans RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT artbataev@gmail.com)

include(CPack)
