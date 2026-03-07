function(sidekick_configure_clang_tools target source_list)
  find_program(CLANG_FORMAT_BIN NAMES clang-format)

  set(CLANG_TIDY_CANDIDATES clang-tidy /opt/homebrew/opt/llvm/bin/clang-tidy)
  find_program(CLANG_TIDY_BIN NAMES ${CLANG_TIDY_CANDIDATES})

  set(_format_files "")
  foreach(_src IN LISTS source_list)
    list(APPEND _format_files "${CMAKE_SOURCE_DIR}/${_src}")
  endforeach()

  if(CLANG_FORMAT_BIN AND _format_files)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_BIN} -i ${_format_files}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting source files"
            VERBATIM
        )

    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_BIN} -n --Werror ${_format_files}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Checking source formatting"
            VERBATIM
        )
  endif()

  if(SIDEKICK_ENABLE_CLANG_TIDY AND CLANG_TIDY_BIN)
    add_custom_target(tidy
            COMMAND ${CLANG_TIDY_BIN} --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy -p=${CMAKE_BINARY_DIR} ${_format_files}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy"
            VERBATIM
        )
  endif()
endfunction()
