#
cmake_minimum_required(VERSION 3.8)


#
project("crypto-exchange-client-huobi")

if (MSVC)
	add_compile_options(-D_WIN32_WINNT=0x0601)
endif()


#
##
set(Boost_MULTITHREADED ON)

find_package(Boost REQUIRED system json iostreams)
include_directories(
	${Boost_INCLUDE_DIRS}
)

##
find_package(OpenSSL)

include_directories(
	${OPENSSL_INCLUDE_DIR}
)

##
include_directories("lib/crypto-exchange-client-core/include")

include_directories("include")


#
add_subdirectory("lib/crypto-exchange-client-core")

add_subdirectory("src/crypto-exchange-client-huobi")
add_subdirectory("src/crypto-exchange-client-huobi-demo")
