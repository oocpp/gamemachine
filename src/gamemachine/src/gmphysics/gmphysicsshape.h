﻿#ifndef __GMPHYSICSSHAPE_H__
#define __GMPHYSICSSHAPE_H__
#include <gmcommon.h>
#include "gmbulletforward.h"
#include "gmphysicsshape.h"

struct GMVec3;
BEGIN_NS

GM_PRIVATE_OBJECT(GMPhysicsShape)
{
	GMOwnedPtr<btCollisionShape> shape;
};

class GMPhysicsShape : public GMObject
{
	GM_DECLARE_PRIVATE(GMPhysicsShape);

	friend struct GMPhysicsShapeCreator;

private:
	GMPhysicsShape() = default;

public:
	~GMPhysicsShape() = default;

public:
	void setShape(btCollisionShape* shape);
	btCollisionShape* getBulletShape();
	const btCollisionShape* getBulletShape() const;
};

struct GMPhysicsShapeCreator
{
	static void createBoxShape(const GMVec3& halfExtents, OUT GMPhysicsShape** shape);
	static void createMeshFromShape(GMPhysicsShape* shape, GMGameObject* gameObject, GMGameWorld* world);
};

END_NS
#endif