# only generate test when the current build is not release
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")

    find_package(Python3 COMPONENTS Interpreter)
    find_program(VIRTUALENV virtualenv)

    # only generate test when python3 and virtualenv is found
    if (Python3_Interpreter_FOUND AND VIRTUALENV)

        # create virtual environment
        add_custom_command(
                OUTPUT venv
                COMMAND ${VIRTUALENV} -p python3 venv
        )

        # set variables needed for building tests
        set(VENV_BIN ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/)
        set(PYTEST ${VENV_BIN}/python ${VENV_BIN}/pytest)
        set(ENV_VARS ${CMAKE_COMMAND} -E env BUILD_DIR=${CMAKE_BINARY_DIR}
                SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                )

        # install requirements
        add_custom_command(
                OUTPUT venv.stamp
                DEPENDS venv requirements.txt
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt requirements.txt
                COMMAND ${VENV_BIN}/python -m pip install --upgrade pip
                COMMAND ${VENV_BIN}/pip install -r requirements.txt --upgrade
        )
        # add these commands to target "tests"
        add_custom_target(tests ALL
                DEPENDS venv.stamp ${PROJECT_NAME}
                SOURCES requirements.txt
                )

        # add all tests
        file(GLOB pytest_files "${CMAKE_CURRENT_SOURCE_DIR}/test_*.py")
        foreach(filepath ${pytest_files})
            get_filename_component(filename ${filepath} NAME_WE)
            add_test(NAME ${filename} COMMAND ${ENV_VARS} ${PYTEST} ${filepath})
        endforeach()

    else ()
        message("Could not find Python3 or Virtualenv for pytest!")
    endif ()

endif()

