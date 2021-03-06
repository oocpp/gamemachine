﻿#include "stdafx.h"
#include "gmdemogameworld.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gm2dgameobject.h"

GMDemoGameWorld::GMDemoGameWorld(const IRenderContext* context)
	: GMGameWorld(context)
{
}

GMDemoGameWorld::~GMDemoGameWorld()
{
	D(d);
	GMClearSTLContainer(d->renderList);
}

bool GMDemoGameWorld::addObject(const GMString& name, GMGameObject* obj)
{
	D(d);
	auto r = d->renderList.find(name);
	if (r != d->renderList.end())
		return false;
	d->renderList[name] = obj;
	d->renderListInv[obj] = name;
	addToRenderList(obj);
	addObjectAndInit(obj);
	return true;
}

bool GMDemoGameWorld::removeObject(const GMString& name)
{
	D(d);
	GMGameObject* object = findGameObject(name);
	if (object)
	{
		d->renderList.erase(name);
		d->renderListInv.erase(object);
		if (object->canDeferredRendering())
			std::remove(getRenderList().deferred.begin(), getRenderList().deferred.end(), object);
		else
			std::remove(getRenderList().forward.begin(), getRenderList().forward.end(), object);
		return GMGameWorld::removeObject(object);
	}
	return false;
}

GMGameObject* GMDemoGameWorld::findGameObject(const GMString& name)
{
	D(d);
	auto target = d->renderList.find(name);
	if (target == d->renderList.end())
		return nullptr;
	return target->second;
}

bool GMDemoGameWorld::findGameObject(const GMGameObject* obj, REF GMString& name)
{
	D(d);
	auto target = d->renderListInv.find(obj);
	if (target == d->renderListInv.end())
		return false;
	name = target->second;
	return true;
}

bool GMDemoGameWorld::removeObject(GMGameObject* obj)
{
	GMString name;
	bool found = findGameObject(obj, name);
	if (found)
	{
		return removeObject(name);
	}
	return false;
}
