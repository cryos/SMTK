# set up sources to build
set(modelSrcs
  Arrangement.cxx
  ArrangementKind.cxx
  AttributeAssignments.cxx
  AttributeListPhrase.cxx
  BRepModel.cxx
  Bridge.cxx
  BridgeSession.cxx
  BridgeIOJSON.cxx
  BridgeRegistrar.cxx
  CellEntity.cxx
  Chain.cxx
  Cursor.cxx
  CursorArrangementOps.cxx
  DefaultBridge.cxx
  DescriptivePhrase.cxx
  Edge.cxx
  EdgeUse.cxx
  Entity.cxx
  EntityListPhrase.cxx
  EntityPhrase.cxx
  Face.cxx
  FaceUse.cxx
  GroupEntity.cxx
  GridInfo.cxx
  InstanceEntity.cxx
  Loop.cxx
  ModelEntity.cxx
  Operator.cxx
  PropertyListPhrase.cxx
  PropertyValuePhrase.cxx
  RemoteOperator.cxx
  Shell.cxx
  ShellEntity.cxx
  SimpleModelSubphrases.cxx
  Manager.cxx
  SubphraseGenerator.cxx
  Tessellation.cxx
  UseEntity.cxx
  Vertex.cxx
  VertexUse.cxx
  Volume.cxx
  VolumeUse.cxx
  operators/SetProperty.cxx
  )

set(modelHeaders
  Arrangement.h
  ArrangementKind.h
  AttributeAssignments.h
  AttributeListPhrase.h
  BRepModel.h
  Bridge.h
  BridgeSession.h
  BridgeIO.h
  BridgeIOJSON.h
  BridgeRegistrar.h
  CellEntity.h
  Chain.h
  Cursor.h
  CursorArrangementOps.h
  DefaultBridge.h
  DescriptivePhrase.h
  Edge.h
  EdgeUse.h
  Entity.h
  EntityListPhrase.h
  EntityPhrase.h
  EntityTypeBits.h
  Events.h
  Face.h
  FaceUse.h
  FloatData.h
  GridInfo.h
  GroupEntity.h
  InstanceEntity.h
  IntegerData.h
  Loop.h
  ModelEntity.h
  Operator.h
  PropertyType.h
  PropertyListPhrase.h
  PropertyValuePhrase.h
  RemoteOperator.h
  Shell.h
  ShellEntity.h
  SimpleModelSubphrases.h
  Manager.h
  SubphraseGenerator.h
  StringData.h
  Tessellation.h
  UseEntity.h
  Vertex.h
  VertexUse.h
  Volume.h
  VolumeUse.h
  operators/SetProperty.h
  )

smtk_bridge_json("${CMAKE_CURRENT_SOURCE_DIR}/DefaultBridge.json" defBridgeJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetProperty.sbt" defOpXML)

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
