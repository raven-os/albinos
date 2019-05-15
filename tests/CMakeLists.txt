add_executable(service-test)
target_sources(service-test PUBLIC service-test.cpp ../vendor/loguru/loguru.cpp)
target_include_directories(service-test PRIVATE ../vendor/doctest/doctest ../vendor/json/single_include/nlohmann ../service  ../vendor/strong_type/include/ ../vendor/expected ../vendor/sql/hdr ../vendor/loguru)
target_link_libraries(service-test albinos::uvw ${sqlite3_lib} stdc++fs)
target_compile_options(service-test PUBLIC -Wfatal-errors -Wall -Wextra -ggdb -g3 -O0)
##sanitizer -> -fsanitize=undefined
##coverage -> --coverage -fprofile-arcs -ftest-coverage
#target_link_options(service-test PUBLIC --coverage -fsanitize=undefined)