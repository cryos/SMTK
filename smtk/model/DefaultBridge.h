//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_DefaultBridge_h
#define __smtk_model_DefaultBridge_h

#include "smtk/model/Bridge.h"

namespace smtk {
  namespace model {

/**\brief A bridge that does no transcription.
  *
  * Bridges of this type do no transcription for one of two reasons:
  * 1. They hold "native" SMTK models, or
  * 2. They forward all requests to a remote bridge (which they
  *    "back" with local storage).
  *
  * Instances of this bridge report their name() as "native".
  * In the first case, they have an empty remoteName().
  * In the second, their remoteName() will be the type of the
  * bridge they provide backing storage for.
  *
  * SMTK does not provide a protocol or transport for forwarding requests.
  * However, it does provide mechanisms for serialization and deserialization
  * of storage, operators, and operator results.
  * Since Bridge instances are the interface between modeling kernels such
  * as OpenCascade or ACIS, this makes them the place where requests must
  * be forwarded if they are going to be.
  *
  * By default this bridge will create instances of RemoteOperator
  * when asked for an Operator.
  * The RemoteOperator class calls virtual methods on DefaultBridge
  * in order to perform operations remotely (i.e., DefaultBridge acts
  * as a delegate for operators.)
  * Subclasses which inherit DefaultBridge in order to provide request
  * forwarding must implement transcribeInternal, ableToOperateDelegate,
  * and operateDelegate methods.
  *
  * See the unitDefaultBridge test for an example of how
  * forwarding works.
  *
  * See the Remus remote bridge for an implementation that can
  * forward requests to Remus workers.
  */
class SMTKCORE_EXPORT DefaultBridge : public Bridge
{
public:
  smtkTypeMacro(DefaultBridge);
  smtkSuperclassMacro(Bridge);
  smtkSharedFromThisMacro(Bridge);
  smtkCreateMacro(DefaultBridge);
  smtkDeclareModelingKernel();

  void backsRemoteBridge(
    const std::string& remoteBridgeName,
    const smtk::common::UUID& bridgeSessionId);
  virtual std::string remoteName() const;
  virtual OperatorPtr op(const std::string& opName) const;

protected:
  friend class RemoteOperator;
  friend class io::ImportJSON;

  DefaultBridge();

  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags);

  virtual bool ableToOperateDelegate(RemoteOperatorPtr op);
  virtual OperatorResult operateDelegate(RemoteOperatorPtr op);

  void setImportingOperators(bool isImporting);

  std::string m_remoteBridgeName;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_DefaultBridge_h
