//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_InstanceEntity_h
#define __smtk_model_InstanceEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

/**\brief A cursor subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT InstanceEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(InstanceEntity,Cursor,isInstanceEntity);

  Cursor prototype() const;

  // InstanceEntity& setTransform(const smtk::common::Matrix4d&);
  // smtk::common::Matrix4d transform() const;
};

typedef std::vector<InstanceEntity> InstanceEntities;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_InstanceEntity_h
