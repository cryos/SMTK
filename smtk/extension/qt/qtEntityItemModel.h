//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_qt_qtEntityItemModel_h
#define __smtk_qt_qtEntityItemModel_h

#include "QAbstractItemModel"
#include "QIcon"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/QtSMTKExports.h" // For EXPORT macro.
#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

/**\brief Adapt an smtk::model::Manager instance into a hierarchical Qt model.
  *
  * This is done by generating instances of smtk::model::DescriptivePhrase
  * subclasses to portray the entities in the model manager both in terms of
  * their inherent attributes and their relations to other entities.
  *
  * By calling setPhrases() on the model, you identify the toplevel
  * description you wish to present; it may cover any subset of
  * an underlying model manager and may even describe entities from different
  * model managers.
  *
  * You may also call setPhraseFilter() on the model with a filter.
  * The filter is used to alter the available subphrases of each
  * descriptive phrase for presentation. For instance, you may write a
  * filter that omits descriptions of attributes on model items.
  */
class QTSMTK_EXPORT QEntityItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QEntityItemModel(QObject* parent = 0);
  virtual ~QEntityItemModel();

  void clear();

  /// Enumeration of model-specific data roles.
  enum DataRoles {
    TitleTextRole             = Qt::UserRole + 100, //!< Entity name (user-editable)
    SubtitleTextRole          = Qt::UserRole + 101, //!< Entity type description
    EntityIconRole            = Qt::UserRole + 102, //!< Entity type icon
    EntityColorRole           = Qt::UserRole + 103, //!< Per-entity color
    EntityVisibilityRole      = Qt::UserRole + 104  //!< Entity visibility
  };

  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual bool hasChildren(const QModelIndex& parent) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& inParent = QModelIndex()) const { (void)inParent; return 1; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  //virtual bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  virtual bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  //virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void setRoot(DescriptivePhrasePtr root)
    {
    this->m_root = root;
    this->updateObserver();
    }

  smtk::model::ManagerPtr manager() const;

  void setDeleteOnRemoval(bool del)
    {
    this->m_deleteOnRemoval = del;
    }

  static QIcon lookupIconForEntityFlags(smtk::model::BitFlags flags);

  DescriptivePhrasePtr getItem(const QModelIndex& idx) const;

  template<typename T, typename C>
  bool foreach_phrase(T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true);
  template<typename T, typename C>
  bool foreach_phrase(T& visitor, C& collector, const QModelIndex& top = QModelIndex(), bool onlyBuilt = true) const;

  void subphrasesUpdated(const QModelIndex& qidx);

protected:
  smtk::model::DescriptivePhrasePtr m_root;
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?
  class Internal;
  Internal* P;

  void updateObserver();
  //template<typename T>
  //void sortDataWithContainer(T& sorter, Qt::SortOrder order);
};

/**\brief Iterate over all expanded entries in the tree.
  *
  */
template<typename T, typename C>
bool QEntityItemModel::foreach_phrase(T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt)
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
    {
    DescriptivePhrasePtr phrase = this->getItem(top);
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some models
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
      {
      for (int row = 0; row < this->rowCount(top); ++row)
        {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
        }
      }
    }
  return false;
}

/// A const version of foreach_phrase. See the non-const version for documentation.
template<typename T, typename C>
bool QEntityItemModel::foreach_phrase(T& visitor, C& collector, const QModelIndex& top, bool onlyBuilt) const
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
    {
    DescriptivePhrasePtr phrase = this->getItem(top);
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some models
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
      {
      for (int row = 0; row < this->rowCount(top); ++row)
        {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
        }
      }
    }
  return false;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_qt_qtEntityItemModel_h
