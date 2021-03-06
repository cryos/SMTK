//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_BridgeIOJSON_h
#define __smtk_model_BridgeIOJSON_h

#include "smtk/model/BridgeIO.h"

struct cJSON;

namespace smtk {
  namespace model {

/**\brief A base class for delegating bridge I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
class SMTKCORE_EXPORT BridgeIOJSON : public BridgeIO
{
public:
  smtkTypeMacro(BridgeIOJSON);

  virtual int importJSON(ManagerPtr modelMgr, cJSON* sessionRec);
  virtual int exportJSON(ManagerPtr modelMgr, cJSON* sessionRec);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BridgeIOJSON_h
