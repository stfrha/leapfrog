LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../Box2D/Box2D/

#SDK_ROOT points to folder with SDL and oxygine-framework
LOCAL_SRC_FILES := ../../../..//SDL/src/main/android/SDL_android_main.c

LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2BroadPhase.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2CollideCircle.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2CollideEdge.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2CollidePolygon.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2Collision.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2Distance.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2DynamicTree.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/b2TimeOfImpact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/Shapes/b2ChainShape.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/Shapes/b2CircleShape.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/Shapes/b2EdgeShape.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Collision/Shapes/b2PolygonShape.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2BlockAllocator.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2Draw.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2Math.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2Settings.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2StackAllocator.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Common/b2Timer.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2Body.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2ContactManager.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2Fixture.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2Island.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2World.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/b2WorldCallbacks.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2Contact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2GearJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2Joint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2MotorJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.cpp
LOCAL_SRC_FILES += ../../../../Box2D/Box2D/Box2D/Rope/b2Rope.cpp
LOCAL_SRC_FILES += ../../../src/asteroid.cpp
LOCAL_SRC_FILES += ../../../src/asteroidfield.cpp
LOCAL_SRC_FILES += ../../../src/blastemitter.cpp
LOCAL_SRC_FILES += ../../../src/blastparticle.cpp
LOCAL_SRC_FILES += ../../../src/Box2DDebugDraw.cpp
LOCAL_SRC_FILES += ../../../src/bullet.cpp
LOCAL_SRC_FILES += ../../../src/compoundinterface.cpp
LOCAL_SRC_FILES += ../../../src/compoundobject.cpp
LOCAL_SRC_FILES += ../../../src/flameemitter.cpp
LOCAL_SRC_FILES += ../../../src/flameparticle.cpp
LOCAL_SRC_FILES += ../../../src/flamesmokeparticle.cpp
LOCAL_SRC_FILES += ../../../src/freespaceactor.cpp
LOCAL_SRC_FILES += ../../../src/freespacecontactlistener.cpp
LOCAL_SRC_FILES += ../../../src/gun.cpp
LOCAL_SRC_FILES += ../../../src/landercontactlistener.cpp
LOCAL_SRC_FILES += ../../../src/landingpad.cpp
LOCAL_SRC_FILES += ../../../src/launchsite.cpp
LOCAL_SRC_FILES += ../../../src/mainactor.cpp
LOCAL_SRC_FILES += ../../../src/landingactor.cpp
LOCAL_SRC_FILES += ../../../src/leapfrog.cpp
LOCAL_SRC_FILES += ../../../src/marker.cpp
LOCAL_SRC_FILES += ../../../src/objectfactory.cpp
LOCAL_SRC_FILES += ../../../src/orbitscene.cpp
LOCAL_SRC_FILES += ../../../src/orbitspacescene.cpp
LOCAL_SRC_FILES += ../../../src/orbitwindow.cpp
LOCAL_SRC_FILES += ../../../src/physdispconvert.cpp
LOCAL_SRC_FILES += ../../../src/planetactor.cpp
LOCAL_SRC_FILES += ../../../src/polygonvertices.cpp
LOCAL_SRC_FILES += ../../../src/reentryflameemitter.cpp
LOCAL_SRC_FILES += ../../../src/reentryflameparticle.cpp
LOCAL_SRC_FILES += ../../../src/reentrysceneactor.cpp
LOCAL_SRC_FILES += ../../../src/scales.cpp
LOCAL_SRC_FILES += ../../../src/sceneactor.cpp
LOCAL_SRC_FILES += ../../../src/shield.cpp
LOCAL_SRC_FILES += ../../../src/smokeparticle.cpp
LOCAL_SRC_FILES += ../../../src/softboundary.cpp
LOCAL_SRC_FILES += ../../../src/main.cpp


LOCAL_STATIC_LIBRARIES := oxygine-framework_static
LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)


#import from NDK_MODULE_PATH defined in build.cmd
$(call import-module, oxygine-framework)
