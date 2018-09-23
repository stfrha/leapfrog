#include "compoundinterface.h"
#include "compoundobject.h"

using namespace std;

CompoundInterface::CompoundInterface()
{

}

CompoundInterface::~CompoundInterface()
{

}


void CompoundInterface::splitString(const string& source, string& thisLevel, string& lowerLevels)
{
   thisLevel = source.substr(0, source.find("."));
   if (thisLevel.size() < source.size())
   {
      lowerLevels = source.substr(thisLevel.size() + 1);
   }
}


/*
Used to find pointers to CompoundObjects as children of this compound object.
name string can be hierarcical like so: nameGrandPareent.nameParent.nameChild
If the name string is hierarchical, all but the last name in the string
are CompoundObjects
*/
CompoundObject* CompoundInterface::getObject(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getObjectImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getObject(lowerLevels);
      }
   }

   return NULL;
}

/*
Used to find pointers to b2Bodies as children of this compound object.
name string can be hierarcical like so: nameGrandPareent.nameParent.nameChild
*/
b2Body* CompoundInterface::getBody(const string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getBodyImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getBody(lowerLevels);
      }
   }

   return NULL;
}

b2Joint* CompoundInterface::getJoint(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getJointImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getJoint(lowerLevels);
      }
   }

   return NULL;
}

System* CompoundInterface::getSystem(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getSystemImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getSystem(lowerLevels);
      }
   }

   return NULL;
}

