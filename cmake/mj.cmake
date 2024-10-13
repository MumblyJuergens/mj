# A lot of this document is courtesy of Jason Turner (https://github.com/lefticus)

set (MUMBLY_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

include("${MUMBLY_CURRENT_LIST_DIR}/CPM.cmake")

macro(MJEnableIPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(SEND_ERROR "IPO is not supported: ${output}")
    endif()    
endmacro()


function(MJFullCompilerWarnings target_name)
    
    if (MSVC)
        target_compile_options(${target_name} INTERFACE
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
        # be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
        # the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
        /WX
        )
    else()
        target_compile_options(${target_name} INTERFACE
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wpedantic # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation)
        -Werror
        )
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target_name} INTERFACE
            -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast # warn if you perform a cast to the same type
            -Wsuggest-override # warn if an overridden member function is not marked 'override' or 'final'
            )
        endif()
    endif()        
endfunction()

# MJLibrarySetup(target INCLUDE_DIR includedir [EXPORT_PREFIX prefix] HEADERS header0 ... headerN)
function(MJLibrarySetup target_name)
    set(options)
    set(oneValueArgs INCLUDE_DIR EXPORT_HEADER_PREFIX)
    set(multiValueArgs HEADERS)
    cmake_parse_arguments(PARSE_ARGV 1 mjls "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set_target_properties(${target_name} PROPERTIES PUBLIC_HEADER "${mjls_HEADERS}" VISIBILITY_INLINES_HIDDEN YES CXX_VISIBILITY_PRESET hidden)
    target_include_directories(${target_name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${mjls_INCLUDE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>/${mjls_INCLUDE_DIR}
        $<INSTALL_INTERFACE:${mjls_INCLUDE_DIR}>
    )
    if(NOT EXPORT_HEADER_PREFIX IN_LIST mjls_KEYWORDS_MISSING_VALUES)
        message("Generating ${mjls_INCLUDE_DIR}/${mjls_EXPORT_HEADER_PREFIX}_export.h ...")
        include(GenerateExportHeader)
        generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "${mjls_INCLUDE_DIR}/${mjls_EXPORT_HEADER_PREFIX}_export.h")
    endif()
endfunction()

function(MJLibraryInstallSetup target_name)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/${target_name}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}Config.cmake
        INSTALL_DESTINATION  "${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}"
    )
    install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${target_name}Config.cmake"
      DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    )

    write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${target_name}ConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion
    )
      
    install(TARGETS ${target_name}
        EXPORT ${target_name}Targets
            COMPONENT ${target_name}-lib
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER
            COMPONENT ${target_name}-dev
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${target_name}
    )
    export(EXPORT ${target_name}Targets FILE ${CMAKE_CURRENT_BINARY_DIR}/${target_name}Targets.cmake)

    install(EXPORT ${target_name}Targets
        NAMESPACE ${target_name}::
        COMPONENT ${target_name}-dev
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}ConfigVersion.cmake
        COMPONENT ${target_name}-dev
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}
    )
endfunction()