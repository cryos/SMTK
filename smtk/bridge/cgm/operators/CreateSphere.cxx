//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateSphere.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "CGMApp.hpp"
#include "DagType.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "InitCGMA.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"
#include "Body.hpp"

#include "smtk/bridge/cgm/CreateSphere_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool CreateSphere::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult CreateSphere::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr centerItem =
    this->specification()->findDouble("center");
  smtk::attribute::DoubleItem::Ptr radiusItem =
    this->specification()->findDouble("radius");
  smtk::attribute::DoubleItem::Ptr innerRadiusItem =
    this->specification()->findDouble("inner radius");

  double center[3];
  double radius = radiusItem->value();
  double innerRadius = innerRadiusItem->value();
  for (int i = 0; i < 3; ++i )
    center[i] = centerItem->value(i);

  //smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  Body* cgmBody = GeometryModifyTool::instance()->sphere(radius, 0., 0., 0., innerRadius);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (!cgmBody)
    {
    smtkInfoMacro(log(), "Failed to create body.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Do this separately because CGM's sphere() method is broken (for OCC at a minimum).
  CubitVector delta(center[0],center[1],center[2]);
  CubitStatus status = GeometryQueryTool::instance()->translate(cgmBody, delta, center[2]);
  if (status != CUBIT_SUCCESS)
    {
    smtkInfoMacro(log(), "Failed to translate body.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("entities");

  Bridge* bridge = this->cgmBridge();
  resultBodies->setNumberOfValues(1);

  smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
  smtk::common::UUID entId = refId->entityId();
  smtk::model::Cursor smtkEntry(this->manager(), entId);
  if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
    resultBodies->setValue(0, smtkEntry);

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::CreateSphere,
  cgm_create_sphere,
  "create sphere",
  CreateSphere_xml,
  smtk::bridge::cgm::Bridge);
