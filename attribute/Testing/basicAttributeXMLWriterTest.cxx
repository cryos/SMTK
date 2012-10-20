/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "attribute/Manager.h"
#include "attribute/Definition.h"
#include "attribute/Attribute.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/DirectoryItemDefinition.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/VoidItemDefinition.h"
#include "attribute/AttributeWriter.h"

#include <iostream>

int main(int argc, char *argv[])
{
  int status;
  {
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " filename\n";
    return -1;
    }
  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets add some analyses
  std::set<std::string> analysis;
  analysis.insert("Flow");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow", analysis);
  analysis.clear();

  analysis.insert("Flow");
  analysis.insert("Heat");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow with Heat Transfer", analysis);
  analysis.clear();

  analysis.insert("Constituent");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("Constituent Transport", analysis);
  analysis.clear();

  // Lets create an attribute to represent an expression
  slctk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  slctk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<slctk::StringItemDefinitionPtr>("Expression String");
  slctk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<slctk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  slctk::IntItemDefinitionPtr iitemdef = 
    base->addItemDefinition<slctk::IntItemDefinitionPtr>("TEMPORAL");
  iitemdef->setCommonValueLabel("Time");
  iitemdef->addDiscreteValue(0, "Seconds");
  iitemdef->addDiscreteValue(1, "Minutes");
  iitemdef->addDiscreteValue(2, "Hours");
  iitemdef->addDiscreteValue(3, "Days");
  iitemdef->setDefaultDiscreteIndex(0);
  iitemdef->addCategory("Time");
  iitemdef = 
    base->addItemDefinition<slctk::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCategory("Heat");

  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
  def1->setAssociationMask(0x20); // belongs on domains
   // Lets add some item definitions
  slctk::DoubleItemDefinitionPtr ditemdef = 
    def1->addItemDefinition<slctk::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->addCategory("Veg");
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
    {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
    }
  ditemdef = 
    def1->addItemDefinition<slctk::DoubleItemDefinitionPtr>("DoubleItem2");
  ditemdef->setDefaultValue(-35.2);
  ditemdef->setMinRange(-100, true);
  ditemdef->setMaxRange(125.0, false);
  ditemdef->addCategory("Constituent");
  slctk::VoidItemDefinitionPtr vdef = 
    def1->addItemDefinition<slctk::VoidItemDefinitionPtr>("VoidItem");
  vdef->setIsOptional(true);
  vdef->setLabel("Option 1");
 

  slctk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
  def2->setAssociationMask(0x7);
   // Lets add some item definitions
  slctk::StringItemDefinitionPtr sitemdef = 
    def2->addItemDefinition<slctk::StringItemDefinitionPtr>("StringItem1");
  sitemdef->setIsMultiline(true);
  sitemdef->addCategory("Flow");
  sitemdef = 
    def2->addItemDefinition<slctk::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->addCategory("General");
  slctk::DirectoryItemDefinitionPtr dirdef =
    def2->addItemDefinition<slctk::DirectoryItemDefinitionPtr>("DirectoryItem");
  dirdef->setShouldExist(true);
  dirdef->setShouldBeRelative(true);
  slctk::FileItemDefinitionPtr fdef =
    def2->addItemDefinition<slctk::FileItemDefinitionPtr>("FileItem");
  fdef->setShouldBeRelative(true);
  slctk::GroupItemDefinitionPtr gdef1, gdef =
    def2->addItemDefinition<slctk::GroupItemDefinitionPtr>("GroupItem");
  gdef->addItemDefinition<slctk::FileItemDefinitionPtr>("File1");
  gdef1 = gdef->addItemDefinition<slctk::GroupItemDefinitionPtr>("SubGroup");
  sitemdef = 
    gdef1->addItemDefinition<slctk::StringItemDefinitionPtr>("GroupString");
  sitemdef->setDefaultValue("Something Cool");
  sitemdef->addCategory("General");
  sitemdef->addCategory("Flow");
  // Process Categories
  manager.updateCategories();
  // Lets test creating an attribute by passing in the expression definition explicitly
  slctk::AttributePtr expAtt = manager.createAttribute("Exp1", expDef);
  slctk::AttributePtr att = manager.createAttribute("testAtt", "Derived2");
  if (att == NULL)
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  slctk::ValueItemPtr vitem;
  slctk::AttributeItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = slctk::dynamicCastPointer<slctk::attribute::ValueItem>(item);
  slctk::attribute::AttributeWriter writer;
  if (writer.write(manager, argv[1]))
    {
    std::cerr << "Errors encountered creating Attribute File:\n";
    std::cerr << writer.errorMessages();
    }
  std::cout << "Manager destroyed\n";
  }
  return status;
}
