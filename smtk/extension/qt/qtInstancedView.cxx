//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtInstancedView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/Root.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>
#include <QMessageBox>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtInstancedViewInternals
{
public:
  qtInstancedViewInternals()
    {
    }
  //QScrollArea *ScrollArea;
  QList< QPointer<qtAttribute> > AttInstances;
};

//----------------------------------------------------------------------------
qtInstancedView::
qtInstancedView(smtk::view::BasePtr dataObj, QWidget* p, qtUIManager* uiman) :
  qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtInstancedViewInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtInstancedView::~qtInstancedView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtInstancedView::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  if(this->Widget)
    {
    if(parentlayout)
      {
      parentlayout->removeWidget(this->Widget);
      }
    delete this->Widget;
    }

  this->Widget = new QFrame(this->parentWidget());
/*
  //create the scroll area on the tabs, so we don't make the
  //3d window super small
  this->Internals->ScrollArea = new QScrollArea();
  this->Internals->ScrollArea->setWidgetResizable(true);
  this->Internals->ScrollArea->setFrameShape(QFrame::NoFrame);
  this->Internals->ScrollArea->setObjectName("instancedViewScrollArea");
  this->Internals->ScrollArea->setWidget( this->Widget );
*/
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  //add the advanced layout, and the scroll area onto the
  //widgets to the frame
  if(parentlayout)
    {
    parentlayout->setAlignment(Qt::AlignTop);
    parentlayout->addWidget(this->Widget);
    }

  this->updateAttributeData();
}

//----------------------------------------------------------------------------
void qtInstancedView::updateAttributeData()
{
  smtk::view::InstancedPtr iview =
    smtk::dynamic_pointer_cast<smtk::view::Instanced>(this->getObject());
  if(!iview || !iview->numberOfInstances())
    {
    return;
    }
  foreach(qtAttribute* att, this->Internals->AttInstances)
    {
    this->Widget->layout()->removeWidget(att->widget());
    delete att->widget();
    }
  this->Internals->AttInstances.clear();

  int longLabelWidth = 0;
  std::size_t i, n = iview->numberOfInstances();
  for (i = 0; i < n; i++)
    {
    smtk::attribute::AttributePtr attobj = iview->instance(static_cast<int>(i));
    if(attobj && attobj->numberOfItems()>0)
      {
      int labelWidth = this->uiManager()->getWidthOfAttributeMaxLabel(
        attobj->definition(), this->uiManager()->advancedFont());
      longLabelWidth = std::max(labelWidth, longLabelWidth);
      }
    }
  this->setFixedLabelWidth(longLabelWidth);

  for (i = 0; i < n; i++)
    {
    smtk::attribute::AttributePtr attobj = iview->instance(static_cast<int>(i));
    if(!attobj)
      {
      QMessageBox::warning(this->parentWidget(), tr("Instanced Attribute View"),
      tr("The requested attribute instance does not exist!"));
      }
    else if(attobj->numberOfItems()>0)
      {
      qtAttribute* attInstance = new qtAttribute(attobj, this->widget(), this);
      if(attInstance)
        {
        this->Internals->AttInstances.push_back(attInstance);
        if(attInstance->widget())
          {
          this->Widget->layout()->addWidget(attInstance->widget());
          }
        }
      }
    }
}
//----------------------------------------------------------------------------
void qtInstancedView::showAdvanceLevelOverlay(bool show)
{
  foreach(qtAttribute* att, this->Internals->AttInstances)
    {
    if(att->widget())
      {
      att->showAdvanceLevelOverlay(show);
      }
    }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}
