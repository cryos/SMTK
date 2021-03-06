//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "Bridge.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Tessellation.h"

#include "vtkCellArray.h"
#include "vtkGeometryFilter.h"
#include "vtkHyperTreeGrid.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk {
  namespace bridge {
    namespace exodus {

vtkInformationKeyMacro(Bridge,SMTK_UUID_KEY,String);

enum smtkCellTessRole {
  SMTK_ROLE_VERTS,
  SMTK_ROLE_LINES,
  SMTK_ROLE_POLYS
};

// ++ 2 ++
Bridge::Bridge()
{
  this->initializeOperatorSystem(Bridge::s_operators);
}
// -- 2 --

Bridge::~Bridge()
{
}

// ++ 3 ++
/// Turn any valid cursor into an entity handle.
EntityHandle Bridge::toEntity(const smtk::model::Cursor& eid)
{
  ReverseIdMap_t::const_iterator it = this->m_revIdMap.find(eid);
  if (it == this->m_revIdMap.end())
    return EntityHandle();
  return it->second;
}
// -- 3 --

// ++ 4 ++
smtk::model::Cursor Bridge::toCursor(const EntityHandle& ent)
{
  vtkDataObject* entData = this->toBlock<vtkDataObject>(ent);
  if (!entData)
    return Cursor(); // an invalid cursor

  const char* uuidChar = entData->GetInformation()->Get(SMTK_UUID_KEY());
  smtk::common::UUID uid;
  if (!uuidChar)
    { // We have not assigned a UUID yet. Do so now.
    uid = this->m_uuidGen.random();
    entData->GetInformation()->Set(SMTK_UUID_KEY(), uid.toString().c_str());
    }
  else
    {
    uid = smtk::common::UUID(uuidChar);
    }
  return Cursor(this->manager(), uid);
}
// -- 4 --

// ++ 5 ++
std::vector<EntityHandle> Bridge::childrenOf(const EntityHandle& ent)
{
  std::vector<EntityHandle> children;
  if (ent.entityType != EXO_MODEL)
    return children; // element blocks, side sets, and node sets have no children (yet).

  vtkMultiBlockDataSet* model = this->toBlock<vtkMultiBlockDataSet>(ent);
  if (!model)
    return children;

  struct {
    EntityType entityType;
    int blockId;
  } blocksByType[] = {
    {EXO_BLOCK,    0},
    {EXO_SIDE_SET, 4},
    {EXO_NODE_SET, 7}
  };
  const int numBlocksByType =
    sizeof(blocksByType) / sizeof(blocksByType[0]);
  for (int i = 0; i < numBlocksByType; ++i)
    {
    vtkMultiBlockDataSet* typeSet =
      dynamic_cast<vtkMultiBlockDataSet*>(
        model->GetBlock(blocksByType[i].blockId));
    if (!typeSet) continue;
    for (unsigned j = 0; j < typeSet->GetNumberOfBlocks(); ++j)
      children.push_back(
        EntityHandle(blocksByType[i].entityType, ent.modelNumber, j));
    }
  return children;
}
// -- 5 --

// ++ 6 ++
/// Add the dataset and its blocks to the bridge.
smtk::model::ModelEntity Bridge::addModel(
  vtkSmartPointer<vtkMultiBlockDataSet>& model)
{
  EntityHandle handle;
  handle.modelNumber = static_cast<int>(this->m_models.size());
  handle.entityType = EXO_MODEL;
  handle.entityId = -1; // unused for EXO_MODEL.
  this->m_models.push_back(model);
  smtk::model::ModelEntity result = this->toCursor(handle);
  this->m_revIdMap[result] = handle;
  this->transcribe(result, smtk::model::BRIDGE_EVERYTHING, false);
  this->manager()->setBridgeForModel(shared_from_this(), result.entity());
  return result;
}
// -- 6 --

// ++ 7 ++
BridgedInfoBits Bridge::transcribeInternal(
  const smtk::model::Cursor& entity,
  BridgedInfoBits requestedInfo)
{
  BridgedInfoBits actual = BRIDGE_NOTHING;
  EntityHandle handle = this->toEntity(entity);
  if (!handle.isValid())
    return actual;

  vtkDataObject* obj = this->toBlock<vtkDataObject>(handle);
  // ...
// -- 7 --
  if (!obj)
    return actual;

  int dim = obj->GetInformation()->Get(vtkHyperTreeGrid::DIMENSION());

  // Grab the parent entity early if possible... we need its dimension().
  Cursor parentCursor;
  EntityHandle parentHandle = handle.parent();
  if (parentHandle.isValid())
    {
    parentCursor = this->toCursor(parentHandle);
    if (!parentCursor.isValid())
      {
      // The handle is valid, so perhaps we were asked to
      // transcribe a group before its parent model?
      this->declareDanglingEntity(parentCursor, 0);
      this->transcribe(parentCursor, requestedInfo, true);
      }
    dim = parentCursor.embeddingDimension();
    }

// ++ 8 ++
  smtk::model::Cursor mutableCursor(entity);
  BitFlags entityDimBits;
  if (!mutableCursor.isValid())
    {
// -- 8 --
// ++ 9 ++
    switch (handle.entityType)
      {
    case EXO_MODEL:
      mutableCursor.manager()->insertModel(
        mutableCursor.entity(), dim, dim);
      break;
    case EXO_BLOCK:
      entityDimBits = Entity::dimensionToDimensionBits(dim);
      mutableCursor.manager()->insertGroup(
        mutableCursor.entity(), MODEL_DOMAIN | entityDimBits,
        this->toBlockName(handle));
      mutableCursor.as<GroupEntity>().setMembershipMask(VOLUME);
      break;
    // .. and other cases.
// -- 9 --
    case EXO_SIDE_SET:
      entityDimBits = 0;
      for (int i = 0; i < dim; ++i)
        entityDimBits |= Entity::dimensionToDimensionBits(i);
      mutableCursor.manager()->insertGroup(
        mutableCursor.entity(), MODEL_BOUNDARY | entityDimBits,
        this->toBlockName(handle));
      mutableCursor.as<GroupEntity>().setMembershipMask(CELL_ENTITY | entityDimBits);
      break;
    case EXO_NODE_SET:
      mutableCursor.manager()->insertGroup(
        mutableCursor.entity(), MODEL_BOUNDARY | DIMENSION_0,
        this->toBlockName(handle));
      mutableCursor.as<GroupEntity>().setMembershipMask(VERTEX);
      break;
// ++ 10 ++
    default:
      return actual;
      break;
      }
    actual |= smtk::model::BRIDGE_ENTITY_TYPE;
    }
  else
    {
    // If the entity is valid, is there any reason to refresh it?
    // Perhaps we want additional information transcribed?
    if (this->danglingEntities().find(mutableCursor) ==
      this->danglingEntities().end())
      return smtk::model::BRIDGE_EVERYTHING; // Not listed as dangling => everything transcribed already.
    }
// -- 10 --

// ++ 11 ++
  if (requestedInfo & (smtk::model::BRIDGE_ENTITY_RELATIONS | smtk::model::BRIDGE_ARRANGEMENTS))
    {
    if (parentCursor.isValid())
      { // Connect this entity to its parent.
      mutableCursor.findOrAddRawRelation(parentCursor);
      }
    // Now add children.
    std::vector<EntityHandle> children = this->childrenOf(handle);
    std::vector<EntityHandle>::iterator cit;
    for (cit = children.begin(); cit != children.end(); ++cit)
      {
      Cursor childCursor = this->toCursor(*cit);
      if (!childCursor.isValid())
        {
        this->m_revIdMap[childCursor] = *cit;
        this->declareDanglingEntity(childCursor, 0);
        this->transcribe(childCursor, requestedInfo, true);
        }
      mutableCursor.as<smtk::model::ModelEntity>().addGroup(childCursor);
      }

    // Mark that we added this information to the manager:
    actual |= smtk::model::BRIDGE_ENTITY_RELATIONS | smtk::model::BRIDGE_ARRANGEMENTS;
    }
// -- 11 --
  if (requestedInfo & smtk::model::BRIDGE_ATTRIBUTE_ASSOCIATIONS)
    {
    // FIXME: Todo.
    actual |= smtk::model::BRIDGE_ATTRIBUTE_ASSOCIATIONS;
    }
  if (requestedInfo & smtk::model::BRIDGE_TESSELLATION)
    {
    if (this->addTessellation(entity, handle))
      actual |= smtk::model::BRIDGE_TESSELLATION;
    }
  if (requestedInfo & smtk::model::BRIDGE_PROPERTIES)
    {
    // Set properties.
    actual |= smtk::model::BRIDGE_PROPERTIES;
    }

  this->declareDanglingEntity(mutableCursor, actual);
  return actual;
}

/// Return the block name for the given handle.
std::string Bridge::toBlockName(const EntityHandle& handle) const
{
  if (
    handle.entityType == EXO_INVALID ||
    (handle.entityType != EXO_MODEL && handle.entityId < 0) ||
    handle.modelNumber < 0 ||
    handle.modelNumber > static_cast<int>(this->m_models.size()))
    return NULL;

  int blockId = -1; // Where in the VTK dataset is the entity type data?
  switch (handle.entityType)
    {
  case EXO_MODEL:    return std::string(); break;
  case EXO_BLOCK:    blockId = 0; break;
  case EXO_SIDE_SET: blockId = 4; break;
  case EXO_NODE_SET: blockId = 7; break;
  default:
    return std::string();
    }
  vtkMultiBlockDataSet* typeSet =
    vtkMultiBlockDataSet::SafeDownCast(
      this->m_models[handle.modelNumber]->GetBlock(blockId));
  if (!typeSet || handle.entityId >= typeSet->GetNumberOfBlocks())
    return std::string();
  return std::string(
    typeSet->GetMetaData(handle.entityId)->Get(
      vtkCompositeDataSet::NAME()));
}

// A method that helps convert vtkPolyData into an SMTK Tessellation.
static void AddCellsToTessellation(
  vtkPoints* pts,
  vtkCellArray* cells,
  smtkCellTessRole role,
  std::map<vtkIdType,int>& vertMap,
  smtk::model::Tessellation& tess)
{
  vtkIdType npts;
  vtkIdType* conn;
  std::vector<int> tconn;
  std::map<vtkIdType,int>::iterator pit;
  for (cells->InitTraversal(); cells->GetNextCell(npts, conn); )
    {
    tconn.clear();
    tconn.reserve(npts + 2);
    switch (role)
      {
    case SMTK_ROLE_VERTS:
      if (npts > 1)
        {
        tconn.push_back(TESS_POLYVERTEX);
        tconn.push_back(npts);
        }
      else
        {
        tconn.push_back(TESS_VERTEX);
        }
      break;
    case SMTK_ROLE_LINES:
      tconn.push_back(TESS_POLYLINE);
      tconn.push_back(npts);
      break;
    case SMTK_ROLE_POLYS:
      switch (npts)
        {
      case 0:
      case 1:
      case 2:
        std::cerr
          << "Too few points (" << npts
          << ") for a surface primitive. Skipping.\n";
        continue;
        break;
      case 3: tconn.push_back(TESS_TRIANGLE); break;
      case 4: tconn.push_back(TESS_QUAD); break;
      default: tconn.push_back(TESS_POLYGON); tconn.push_back(npts); break;
        }
      break;
    default:
      std::cerr << "Unknown tessellation role " << role << ". Skipping.\n";
      continue;
      break;
      }
    for (vtkIdType i = 0; i < npts; ++i)
      {
      if ((pit = vertMap.find(conn[i])) == vertMap.end())
        pit = vertMap.insert(
          std::pair<vtkIdType,int>(
            conn[i], tess.addCoords(pts->GetPoint(conn[i])))).first;
      tconn.push_back(pit->second );
      }
    tess.insertNextCell(tconn);
    }
}


bool Bridge::addTessellation(
  const smtk::model::Cursor& cursor,
  const EntityHandle& handle)
{
  if (cursor.hasTessellation())
    return true; // no need to recompute.

  vtkDataObject* data = this->toBlock<vtkDataObject>(handle);
  if (!data)
    return false; // can't squeeze triangles from a NULL

  vtkNew<vtkGeometryFilter> bdyFilter;
  bdyFilter->MergingOff();
  bdyFilter->SetInputDataObject(data);
  bdyFilter->Update();
  vtkPolyData* bdy = bdyFilter->GetOutput();

  if (!bdy)
    return BRIDGE_NOTHING;

  smtk::model::Tessellation tess;
  std::map<vtkIdType,int> vertMap;
  vtkPoints* pts = bdy->GetPoints();
  vtkCellArray* cells;
  cells = bdy->GetVerts();
  AddCellsToTessellation(pts, bdy->GetVerts(), SMTK_ROLE_VERTS, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetLines(), SMTK_ROLE_LINES, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetPolys(), SMTK_ROLE_POLYS, vertMap, tess);
  if (bdy->GetStrips() && bdy->GetStrips()->GetNumberOfCells() > 0)
    {
    std::cerr << "Warning: Triangle strips in discrete cells are unsupported. Ignoring.\n";
    }
  if (!vertMap.empty())
    cursor.manager()->setTessellation(cursor.entity(), tess);

  return true;
}

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

// ++ 1 ++
#include "Bridge_json.h"

smtkImplementsModelingKernel(
  exodus,
  Bridge_json,
  BridgeHasNoStaticSetup,
  smtk::bridge::exodus::Bridge,
  true /* inherit "universal" operators */
);
// -- 1 --
