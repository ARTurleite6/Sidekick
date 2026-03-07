function(sidekick_set_project_warnings target warnings_as_errors)
  if(NOT SIDEKICK_ENABLE_WARNINGS)
    return()
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(warnings_as_errors)
      target_compile_options(${target} PRIVATE /WX)
    endif()
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
        )
  else()
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
        )
  endif()

  if(warnings_as_errors)
    target_compile_options(${target} PRIVATE -Werror)
  endif()
endfunction()
