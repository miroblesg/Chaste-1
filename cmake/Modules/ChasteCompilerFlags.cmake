message("* Adding compiler flags...")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wextra -Wno-unused-parameter -Wvla")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wextra -Wno-unused-parameter -Wvla")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wextra -Wno-unused-parameter -Wno-unused-variable -ftemplate-depth-512")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wextra -Wno-unused-parameter -Wno-unused-variable -ftemplate-depth-512")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
    set(Intel_flags
        # Not available on 10.0
        -Wnon-virtual-dtor -Woverloaded-virtual -Wno-unused-parameter
        
        -wr2304 #2304: non-explicit constructor with single argument
        
        # Switch these ones on for compatibility 
        -wr271 #271: trailing comma is nonstandard

        #Following doesn't seem to play
        -wr810 #810: conversion from "double" to "unsigned int" may lose significant bits

        # This is where the statement is unreachable in a particular instatiation of the template.  e.g. "if (SPACE_DIM<3){return;}" will complain that the SPACE_DIM=3 specific code is unreachable.
       -wr111 #111: statement is unreachable (DUE TO INSTANTIATED TEMPLATES)

        # This is where the statement is unreachable in a particular instatiation of the template.  e.g. "if (ELEMENT_DIM<SPACE_DIM){return;}" will complain that the ELEMENT_DIM == SPACE_DIM dynamic initialization is unreachable.
        -wr185 #185: dynamic initialization in unreachable code (DUE TO INSTANTIATED TEMPLATES)

        # This happens when a switch is based on an unsigned template parameter
        -wr280 #280: selector expression is constant

        # This is seen when used templates to access the is_abstract base class definition
        -wr304 #304: access control not specified ("public" by default)

        # This is when we pass an explict string to a std::string reference: e.g. FileFinder save_bidomain_dir("some_directory", RelativeTo::ChasteSourceRoot);
        -wr383 #383: value copied to temporary, reference to temporary used

        # Noncopyable doesn't have a virtual destructor.  The derived class should not have access to it either
        -wr444 #444: destructor for base class "boost::noncopyable_::noncopyable" ... is not virtual

        # Most commonly seen in archiving where the "version" variable is often redundant
        -wr869 #869: parameter "..." was never referenced

        # Triggered by macros such as TS_ASSERT_EQUALS(a,b)
        -wr981 #981: operands are evaluated in unspecified order

        # We do this when we need to define templated functions in the header file
        -wr1418 #1418: external function definition with no prior declaration

        # There are times when we want a local helper function (RecursiveCopy in FileFinder) or when we need to refer to KSPConvergedReasons
        -wr1419 #1419: external declaration in primary source file

        # This one is potentially useful for telling us where we might want to use CompareDoubles::WithinRelativeTolerance, but in our core code (TimeStepper) the tests should ensure we aren't doing anything silly
        -wr1572 #1572: floating-point equality and inequality comparisons are unreliable

        #2289: proper signature for "auto_ptr" is "Type(const Type&)"

        #2026: Effective C++ Item 14 Make sure base classes have virtual destructors

         #2305: declaration of 'explicit' constructor without a single argument is redundant
        )
    string (REPLACE ";" " " Intel_flags_str "${Intel_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Intel_flags_str}") 
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${Intel_flags_str}") 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #For GUI configs. Change C, and CXX compiler flags dynamically to static, debug build.
    #The overrides.cmake include takes care of non GUI builds.
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
       endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)

    # A bunch of compiler flags. Since we now propagate the compiler and linker flags to all external projects
    # this is one place to set all the flags.
    add_definitions(
        -Z7     # => embed debugging info in library as opposed to using an external .pdb database
        -wd4996 # => disable insecure api warnings
        -wd4267 # => disable "possible loss of data due to 'narrowing' conversion, e.g. size_t to int"
        -wd4290 # => disable warning "C++ exception specification ignored except to indicate a function is not __declspec(nothrow)"
        -wd4005 # => 'identifier' : macro redefinition
        -wd4018 # => 'expression' : signed/unsigned mismatch
        -wd4244 # => 'argument' : conversion from 'type1' to 'type2', possible loss of data
        -wd4101 # => 'identifier' : unreferenced local variable
        -wd4661 # => 'identifier' : no suitable definition provided for explicit template instantiation request
    )

else()
    message(WARNING "Unknown CXX compiler type ${CMAKE_CXX_COMPILER_ID}")
endif()