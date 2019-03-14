add_executable(service-test)
target_sources(service-test PUBLIC service-test.cpp ../vendor/loguru/loguru.cpp)
target_include_directories(service-test PRIVATE ../vendor/doctest/doctest ../vendor/json/single_include/nlohmann ../service ../vendor/uvw/src ../vendor/uvw/deps/libuv/include ../vendor/strong_type/include/ ../vendor/expected ../vendor/sql/hdr ../vendor/loguru)
target_link_libraries(service-test uv_a ${sqlite3_lib})
target_compile_options(service-test PUBLIC -Wfatal-errors -Wall -Wextra)
##sanitizer -> -fsanitize=undefined
##coverage -> --coverage -fprofile-arcs -ftest-coverage
target_link_options(service-test PUBLIC --coverage -fsanitize=undefined)