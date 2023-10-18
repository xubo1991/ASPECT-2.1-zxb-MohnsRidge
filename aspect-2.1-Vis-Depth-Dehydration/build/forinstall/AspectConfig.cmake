# Copyright (C) 2014 - 2018 by the authors of the ASPECT code.
#
# This file is part of ASPECT.
#
# ASPECT is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# ASPECT is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ASPECT; see the file LICENSE.  If not see
# <http://www.gnu.org/licenses/>.


# This file provides a macro that authors can use to
# set up a directory with source files that will then be
# compiled into a run-time loadable plugin for Aspect.


FIND_PACKAGE(deal.II 8.5.0 QUIET REQUIRED HINTS /usr/local/dealii-8.5.1)
SET(Aspect_INCLUDE_DIRS "/usr/local/include")
SET(Aspect_VERSION "2.1.0")
SET(Aspect_DIR "/usr/local/bin")
SET(ASPECT_USE_PETSC "OFF")
# force our build type to the one that is used by ASPECT:
SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "select debug or release mode" FORCE)

MACRO(ASPECT_SETUP_PLUGIN _target)
  MESSAGE(STATUS "Setting up plugin:")
  MESSAGE(STATUS "  name <${_target}>")
  MESSAGE(STATUS "  using ASPECT_DIR ${Aspect_DIR}")
  MESSAGE(STATUS "  in Release mode")
  DEAL_II_SETUP_TARGET(${_target})
  SET_PROPERTY(TARGET ${_target} APPEND PROPERTY
    INCLUDE_DIRECTORIES "${Aspect_INCLUDE_DIRS}"
  )

  IF(ASPECT_USE_PETSC)
    MESSAGE(STATUS "  with PETSC=ON")
    SET_PROPERTY(TARGET ${_target}
      APPEND PROPERTY COMPILE_DEFINITIONS ASPECT_USE_PETSC="1")
  ENDIF() 

  IF(ASPECT_WITH_PERPLEX)
    MESSAGE(STATUS "  with PerpleX=ON")
    SET_PROPERTY(TARGET ${_target}
      APPEND PROPERTY COMPILE_DEFINITIONS ASPECT_WITH_PERPLEX="1")
  ENDIF()

  # export ASPECT_SOURCE_DIR as compile definition
  TARGET_COMPILE_DEFINITIONS( ${_target} PRIVATE ASPECT_SOURCE_DIR="/home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth")

  # workarounds for MAC OS
  IF (APPLE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # avoid linker errors about missing functions inside ASPECT:
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")

    # make shared libraries end on .so not .dylib
    SET_TARGET_PROPERTIES(${_target} PROPERTIES SUFFIX ".so")
  ENDIF()


  # automatically create a symbolic link to aspect in the current directory:
  ADD_CUSTOM_COMMAND(
    TARGET ${_target} POST_BUILD
    COMMAND ln -sf ${Aspect_DIR}/aspect .)

ENDMACRO()
