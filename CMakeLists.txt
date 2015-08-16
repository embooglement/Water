cmake_minimum_required (VERSION 3.1)
project (Water)

set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_C_FLAGS}")
file (GLOB SOURCE_FILES "source/*.cpp")

include_directories ("source")
add_executable (water ${SOURCE_FILES})

set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /usr/local/lib/boost_1_59_0/boost)
set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib/boost_1_59_0/stage/lib)

set (Boost_ADDITIONAL_VERSIONS "1.58" "1.58.0" "1.59" "1.59.0")
set (BOOST_ROOT /usr/local/lib/boost_1_59_0)
set (BOOST_INCLUDEDIR /usr/local/lib/boost_1_59_0/boost)
set (BOOST_LIBRARYDIR /usr/local/lib/boost_1_59_0/stage/lib)

set (Boost_NO_BOOST_CMAKE ON)	
set (BOOST_NO_SYSTEM_PATHS ON)
find_package (Boost COMPONENTS log log_setup thread filesystem system)

if (Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS})

	target_link_libraries (water
		${Boost_LOG_SETUP_LIBRARY}
		${Boost_LOG_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
	)
endif ()

install (FILES vimfiles/syntax.vim DESTINATION ~/.vim/syntax/ RENAME water.vim)
install (FILES vimfiles/ftdetect.vim DESTINATION ~/.vim/ftdetect/ RENAME water.vim)
