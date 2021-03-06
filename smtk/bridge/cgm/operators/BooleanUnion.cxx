//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/BooleanUnion.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
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

#include "smtk/bridge/cgm/BooleanUnion_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool BooleanUnion::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult BooleanUnion::operateInternal()
{
  int keepInputs = this->findInt("keep inputs")->value();
  ModelEntities bodiesIn = this->associatedEntitiesAs<ModelEntities>();

  ModelEntities::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  Body* cgmBody;
  for (it = bodiesIn.begin(); it != bodiesIn.end(); ++it)
    {
    cgmBody = dynamic_cast<Body*>(this->cgmEntity(*it));
    if (cgmBody)
      cgmBodiesIn.append(cgmBody);
    if (!keepInputs)
      this->manager()->eraseModel(*it);
    }

  if (cgmBodiesIn.size() < 2)
    {
    smtkInfoMacro(log(), "Need multiple bodies to union, given " << cgmBodiesIn.size() << ".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  //smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  CubitStatus s = GeometryModifyTool::instance()->unite(cgmBodiesIn, cgmBodiesOut, keepInputs);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (s != CUBIT_SUCCESS)
    {
    smtkInfoMacro(log(), "Failed to perform union (status " << s << ").");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("entities");

  Bridge* bridge = this->cgmBridge();
  int numBodiesOut = cgmBodiesOut.size();
  resultBodies->setNumberOfValues(numBodiesOut);

  for (int i = 0; i < numBodiesOut; ++i)
    {
    cgmBody = cgmBodiesOut.get_and_step();
    if (!cgmBody)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      resultBodies->setValue(i, smtkEntry);
    }

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::BooleanUnion,
  cgm_boolean_union,
  "union",
  BooleanUnion_xml,
  smtk::bridge::cgm::Bridge);
