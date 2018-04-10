#pragma once

#include "oxygine-framework.h"

class b2Body;
class b2Joint;
class CompoundObject;

class CompoundInterface
{
private:
   void splitString(const std::string& source, std::string& thisLevel, std::string& lowerLevels);

public:
   CompoundInterface();

   ~CompoundInterface();

   CompoundObject* getObject(const std::string& name);
   b2Body* getBody(const std::string& name);
   b2Joint* getJoint(const std::string& name);

   virtual b2Body* getBodyImpl(const std::string& name) = 0;
   virtual CompoundObject* getObjectImpl(const std::string& name) = 0;
   virtual b2Joint* getJointImpl(const std::string& name) = 0;
};

