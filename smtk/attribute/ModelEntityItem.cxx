//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

using namespace smtk::attribute;

/// Construct an item given its owning attribute and location in the attribute.
ModelEntityItem::ModelEntityItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

/// Construct an item given its owning item and position inside the item.
ModelEntityItem::ModelEntityItem(
  Item* inOwningItem, int itemPosition, int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

/// Destructor
ModelEntityItem::~ModelEntityItem()
{
}

/// Return the type of storage used by the item.
Item::Type ModelEntityItem::type() const
{
  return MODEL_ENTITY;
}

/// Set the definition of this attribute.
bool ModelEntityItem::setDefinition(
  smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ModelEntityItemDefinition *def =
    dynamic_cast<const ModelEntityItemDefinition *>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
    {
    this->m_values.resize(n);
    }
  return true;
}

/// Return the size of the item (number of entities associated with the item).
std::size_t ModelEntityItem::numberOfValues() const
{
  return this->m_values.size();
}

/// Return the number of values required by this item's definition (if it has one).
std::size_t ModelEntityItem::numberOfRequiredValues() const
{
  const ModelEntityItemDefinition *def =
    static_cast<const ModelEntityItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}

/// Set the number of entities to be associated with this item (returns true if permitted).
bool ModelEntityItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  // Next - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (!def->isExtensible())
    return false; // You may not resize.

  // Next - are we within the prescribed limits?
  std::size_t n = def->numberOfRequiredValues();
  if (newSize < n)
    return false; // The number of values requested is too small.

  n = def->maxNumberOfValues();
  if (n > 0 && newSize > n)
    return false; // The number of values requested is too large.

  this->m_values.resize(newSize);
  return true;
}

/// Return the \a i-th entity stored in this item.
smtk::model::Cursor ModelEntityItem::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(this->m_values.size()))
    return smtk::model::Cursor();
  return this->m_values[i];
}

/**\brief Set the entity stored with this item.
  *
  * This always sets the 0-th item and is a convenience method
  * for cases where only 1 value is needed.
  */
bool ModelEntityItem::setValue(const smtk::model::Cursor& val)
{
  return this->setValue(0, val);
}

/// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
bool ModelEntityItem::setValue(std::size_t i, const smtk::model::Cursor& val)
{
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (def->isValueValid(val))
    {
    this->m_values[i] = val;
    return true;
    }
  return false;
}

bool ModelEntityItem::appendValue(const smtk::model::Cursor& val)
{
  // First - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (
    (def->isExtensible() && def->maxNumberOfValues() && this->m_values.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && this->m_values.size() >= def->numberOfRequiredValues()))
    {
    // The maximum number of values is fixed
    return false;
    }

  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    return true;
    }
  return false;
}

bool ModelEntityItem::removeValue(std::size_t i)
{
  //First - are we allowed to change the number of values?
  const ModelEntityItemDefinition *def =
    static_cast<const ModelEntityItemDefinition *>(this->definition().get());
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }
  this->m_values.erase(this->m_values.begin()+i);
  return true;
}

/// This clears the list of values and then fills it with null entities up to the number of required values.
void ModelEntityItem::reset()
{
  this->m_values.clear();
  if (this->numberOfRequiredValues() > 0)
    this->m_values.resize(this->numberOfRequiredValues());
}

/// A convenience method to obtain the first value in the item as a string.
std::string ModelEntityItem::valueAsString() const
{
  return this->valueAsString(0);
}

/// Return the value of the \a i-th model item as a string. This returns the UUID of the entity.
std::string ModelEntityItem::valueAsString(std::size_t i) const
{
  smtk::model::Cursor val = this->value(i);
  return val.entity().toString();
}

/// Return whether the \a i-th value is set (i.e., a valid model entity).
bool ModelEntityItem::isSet(std::size_t i) const
{
  return i < this->m_values.size() ?
    this->m_values[i].isValid() :
    false;
}

/// Force the \a i-th value of the item to be invalid.
void ModelEntityItem::unset(std::size_t i)
{
  this->setValue(i, smtk::model::Cursor());
}

/// Assigns contents to be same as source item
void ModelEntityItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  Item::copyFrom(sourceItem, info);

  // Cast input pointer to ModelEntityItem
  ModelEntityItemPtr sourceModelEntityItem =
    smtk::dynamic_pointer_cast<ModelEntityItem>(sourceItem);

  // Update values
  // Only set values if both att systems are using the same model
  this->setNumberOfValues(sourceModelEntityItem->numberOfValues());
  for (std::size_t i=0; i<sourceModelEntityItem->numberOfValues(); ++i)
    {
    if (sourceModelEntityItem->isSet(i) && info.IsSameModel)
      {
      smtk::model::Cursor val = sourceModelEntityItem->value(i);
      this->setValue(i, val);
      }
    else
      {
      this->unset(i);
      }
    }
}

/**\brief Return true if the entity is associated with this item; false otherwise.
  *
  */
bool ModelEntityItem::has(const smtk::common::UUID& entity) const
{
  return this->find(entity) >= 0;
}

/**\brief
  *
  */
bool ModelEntityItem::has(const smtk::model::Cursor& entity) const
{
  return this->find(entity) >= 0;
}

/**\brief
  *
  */
smtk::model::CursorArray::const_iterator ModelEntityItem::begin() const
{
  return this->m_values.begin();
}

/**\brief
  *
  */
smtk::model::CursorArray::const_iterator ModelEntityItem::end() const
{
  return this->m_values.end();
}

/**\brief
  *
  */
std::ptrdiff_t ModelEntityItem::find(const smtk::common::UUID& entity) const
{
  std::ptrdiff_t idx = 0;
  smtk::model::CursorArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
    if (it->entity() == entity)
      return idx;
  return -1;
}

/**\brief
  *
  */
std::ptrdiff_t ModelEntityItem::find(const smtk::model::Cursor& entity) const
{
  std::ptrdiff_t idx = 0;
  smtk::model::CursorArray::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
    if (*it == entity)
      return idx;
  return -1;
}
