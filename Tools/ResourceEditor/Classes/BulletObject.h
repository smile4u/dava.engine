/*
 *  BulletObject.h
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#ifndef __BULLET_OBJECT_H__
#define __BULLET_OBJECT_H__

#include "DAVAEngine.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#include "DebugNode.h"

using namespace DAVA;

class BulletObject : public BaseObject
{
public:
    
    BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *meshNode, const Matrix4 &pWorldTransform);
    BulletObject(Scene * scene, btCollisionWorld *collisionWorld, LightNode *lightNode, const Matrix4 &pWorldTransform);
    ~BulletObject();
	
	void UpdateCollisionObject(void);
	
	inline btCollisionObject * GetCollisionObject(void)
	{
		return collisionObject;
	}

	void Draw(const Matrix4 & worldTransform);
	
protected:

	void CreateShape(MeshInstanceNode *meshNode);
	void CreateLightShape(float32 radius);

	btCollisionWorld *collWorld;
	Matrix4 *collisionPartTransform;
	btCollisionObject *collisionObject;
    btTriangleMesh* trimesh;
	btCollisionShape * shape;
	Vector<Vector3> triangles;
};

#endif