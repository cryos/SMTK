//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_Logger_h
#define __smtk_io_Logger_h
/*! \file */

#include "smtk/SMTKCoreExports.h"
#include "smtk/SystemConfig.h"
#include <string>
#include <sstream>
#include <vector>

/// Write the expression \a x (which may use the "<<" operator) to \a logger as an error message.
#define smtkErrorMacro(logger, x) do {                  \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::io::Logger::ERROR,             \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)

/// Write the expression \a x (which may use the "<<" operator) to \a logger as a warning message.
#define smtkWarningMacro(logger, x) do {                \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::io::Logger::WARNING,           \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)

/// Write the expression \a x (which may use the "<<" operator) to \a logger as a debug message.
#define smtkDebugMacro(logger, x) do {                  \
  std::stringstream s1;                                 \
  s1 << x;                                              \
  logger.addRecord(smtk::io::Logger::DEBUG,             \
                   s1.str(),  __FILE__,  __LINE__);     \
  } while (0)
///@}

namespace smtk
{
  namespace io
  {

    /**\brief Log messages for later presentation to a user or a file.
      *
      */
    class SMTKCORE_EXPORT Logger
    {
    public:
      enum Severity
      {DEBUG, INFO, WARNING, ERROR, FATAL};

      struct Record
      {
        Severity severity;
        std::string message;
        std::string fileName;
        unsigned int lineNumber;
        Record(Severity s, const std::string &m,
               const std::string &f="", unsigned int l=0):
          severity(s), message(m), fileName(f), lineNumber(l) {}
        Record():
          severity(INFO), lineNumber(0) {}
      };

      Logger(): m_hasErrors(false) {}
      std::size_t numberOfRecords() const
      {return this->m_records.size();}

      bool hasErrors() const
      {return this->m_hasErrors;}

      void addRecord(Severity s, const std::string &m,
                     const std::string &fname="",
                     unsigned int line=0);

      const Record &record(int i) const
      {return this->m_records[i];}

      // Convert all the messages into a single string
      std::string convertToString() const;
      void reset();

      void append(const Logger &l);

      static std::string severityAsString(Severity s);

    protected:
      std::vector<Record> m_records;
      bool m_hasErrors;
    private:

    };
  };
};

#endif /* __smtk_io_Logger_h */