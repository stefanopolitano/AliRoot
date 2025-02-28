# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

# Module
set(MODULE AliHLTTRD)

# Module include folder
include_directories(${AliRoot_SOURCE_DIR}/HLT/TRD)

# Additional include folders in alphabetical order except ROOT
include_directories(SYSTEM ${ROOT_INCLUDE_DIR})
include_directories(${AliRoot_SOURCE_DIR}/HLT/BASE
                    ${AliRoot_SOURCE_DIR}/HLT/BASE/util
                    ${AliRoot_SOURCE_DIR}/HLT/TPCLib
                    ${AliRoot_SOURCE_DIR}/RAW/RAWDatabase
                    ${AliRoot_SOURCE_DIR}/RAW/RAWDatarec
                    ${AliRoot_SOURCE_DIR}/STEER/CDB
                    ${AliRoot_SOURCE_DIR}/STEER/ESD
                    ${AliRoot_SOURCE_DIR}/STEER/STEER
                    ${AliRoot_SOURCE_DIR}/STEER/STEERBase
                    ${AliRoot_SOURCE_DIR}/TRD/TRDbase
                    ${AliRoot_SOURCE_DIR}/TRD/TRDrec
                   )

# Sources in alphabetical order
set(SRCS
    AliHLTTRDAgent.cxx
    AliHLTTRDCluster.cxx
    AliHLTTRDClusterizer.cxx
    AliHLTTRDDefinitions.cxx
    AliHLTTRDMonitorComponent.cxx
    AliHLTTRDPreprocessorComponent.cxx
    AliHLTTRDTracklet.cxx
    AliTRDonlineTrackingDataContainer.cxx
   )

if(NOT ALIROOT_ONLINE_MODE)
  add_definitions(-DHAVE_ALIGPU -DGPUCA_ALIROOT_LIB)
  include_directories(${AliRoot_SOURCE_DIR}/GPU/GPUTracking/SliceTracker
    ${AliRoot_SOURCE_DIR}/GPU/GPUTracking/TRDTracking
    ${AliRoot_SOURCE_DIR}/GPU/GPUTracking/Merger
    ${AliRoot_SOURCE_DIR}/GPU/GPUTracking/DataTypes
    ${AliRoot_SOURCE_DIR}/GPU/Common
  )
endif()

# Headers from sources
string(REPLACE ".cxx" ".h" HDRS "${SRCS}")
set(HDRS ${HDRS}
    AliHLTTRDTrackletWordArray.h
)

# Generate the dictionary
# It will create G_ARG1.cxx and G_ARG1.h / ARG1 = function first argument
get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary_flat("${MODULE}" "${MODULE}LinkDef.h" "${HDRS}" "${incdirs}")

# Generate the ROOT map
# Dependecies
set(LIBDEPS STEERBase STEER ESD CDB RAWDatabase RAWDatarec HLTbase AliHLTUtil TRDbase TRDrec Net STAT MLP)
generate_rootmap("${MODULE}" "${LIBDEPS}" "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}LinkDef.h")

# Add a library to the project using the specified source files
add_library_tested(${MODULE} SHARED ${SRCS} G__${MODULE}.cxx)
target_link_libraries(${MODULE} ${LIBDEPS})

# Additional compilation flags
set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "")

# System dependent: Modify the way the library is build
if(${CMAKE_SYSTEM} MATCHES Darwin)
    set_target_properties(${MODULE} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(${CMAKE_SYSTEM} MATCHES Darwin)

# Installation
install(TARGETS ${MODULE}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES ${HDRS} DESTINATION include)
