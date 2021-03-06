//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_exodus_Operator_h
#define __smtk_bridge_exodus_Operator_h

#include "smtk/bridge/exodus/SMTKBridgeExodusExports.h"
#include "smtk/model/Operator.h"

class vtkDataObject;

namespace smtk {
  namespace bridge {
    namespace exodus {

class Bridge;
struct EntityHandle;

/**\brief An operator using the Exodus "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing Exodus-specific data
  * for its subclasses to use internally.
  */
class SMTKBRIDGEEXODUS_EXPORT Operator : public smtk::model::Operator
{
protected:
  Bridge* exodusBridge();
  vtkDataObject* exodusData(const smtk::model::Cursor& smtkEntity);
  EntityHandle exodusHandle(const smtk::model::Cursor& smtkEntity);
};

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_exodus_Operator_h
