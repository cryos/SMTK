
################################################################################
# Build cgm bridge
################################################################################
if (SMTK_BUILD_CGM AND CGM_FOUND)
  add_subdirectory(cgm)
endif()

################################################################################
# Build Discrete bridge
################################################################################
if(SMTK_BUILD_DISCRETE_BRIDGE)
  add_subdirectory(discrete)
endif()

################################################################################
# Build Exodus-II bridge
################################################################################
if (SMTK_ENABLE_EXODUS_BRIDGE)
  add_subdirectory(exodus)
endif()

################################################################################
# Build remus remote bridge
################################################################################
# This must come *after* all targets that implement bridges.
if (SMTK_ENABLE_REMUS)
  add_subdirectory(remote)
endif()
