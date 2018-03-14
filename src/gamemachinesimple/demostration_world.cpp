﻿#include "stdafx.h"
#include "demostration_world.h"
#include <gmcontrolgameobject.h>
#include <gmdx11helper.h>
#include <gmcom.h>
#include <gmdxincludes.h>

extern gm::GMRenderEnvironment GetRenderEnv();

DemostrationWorld::~DemostrationWorld()
{
}

void DemostrationWorld::init()
{
	D(d);
	gm::GMGamePackage* package = GM.getGamePackageManager();
	
	// 创建一个带纹理的对象
	gm::GMfloat extents[] = { 1.f, .5f, .5f };
	gm::GMfloat pos[] = { 0, 0, 1 };
	gm::GMModel* model;
	gm::GMPrimitiveCreator::createQuad(extents, pos, &model, nullptr);
	model->getMesh()->getComponents()[0]->getShader().setCull(gm::GMS_Cull::NONE);
	gm::GMAsset asset = gm::GMAssets::createIsolatedAsset(gm::GMAssetType::Model, model);
	d->gameObj = new gm::GMGameObject(asset);
	this->addObjectAndInit(d->gameObj);
}

void DemostrationWorld::renderScene()
{
	D(d);
	Base::renderScene();

	gm::IGraphicEngine* engine = GM.getGraphicEngine();
	engine->drawObjects(&d->gameObj, 1, gm::GMBufferMode::NoFramebuffer);
}

void DemostrationWorld::resetProjectionAndEye()
{
	// 设置一个默认视角
	gm::GMCamera& camera = GM.getCamera();
	camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, -1 };
	lookAt.position = { 0, 0, 1 };
	camera.lookAt(lookAt);
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init()
{
	D(d);
	auto& rc = GM.getGameMachineRunningStates().clientRect;

	gm::GMGamePackage* pk = GM.getGamePackageManager();

#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#endif

	GM.getGraphicEngine()->setShaderLoadCallback(this);

	GMSetRenderState(RENDER_MODE, gm::GMStates_RenderOptions::DEFERRED);
	//GMSetRenderState(EFFECTS, GMEffects::Grayscale);
	GMSetRenderState(RESOLUTION_X, rc.width);
	GMSetRenderState(RESOLUTION_Y, rc.height);

	d->world = new DemostrationWorld();
	d->world->init();
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	getWorld()->resetProjectionAndEye();
}

void DemostrationEntrance::event(gm::GameMachineEvent evt)
{
	D(d);
	gm::IGraphicEngine* engine = GM.getGraphicEngine();
	engine->newFrame();

	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		break;
	case gm::GameMachineEvent::Render:
		getWorld()->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
	{
		getWorld()->notifyControls();

		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();

		if (kbState.keyTriggered('Q') || kbState.keyTriggered(VK_ESCAPE))
			GM.postMessage({ gm::GameMachineMessageType::Quit });

		if (kbState.keyTriggered('B'))
			GM.postMessage({ gm::GameMachineMessageType::Console });

		if (kbState.keyTriggered('L'))
			GMSetDebugState(POLYGON_LINE_MODE, !GMGetDebugState(POLYGON_LINE_MODE));

		if (kbState.keyTriggered('I'))
			GMSetDebugState(RUN_PROFILE, !GMGetDebugState(RUN_PROFILE));

		break;
	}
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}

	if (GM.getCursor())
	{
		if (evt == gm::GameMachineEvent::FrameStart)
			GM.getCursor()->update();
		else if (evt == gm::GameMachineEvent::Render)
			GM.getCursor()->drawCursor();
	}
}

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	gm::GM_delete(d->world);
}

void DemostrationEntrance::onLoadShaders(gm::IGraphicEngine* engine)
{
#if GM_USE_DX11
	HRESULT hr = gm::GMLoadDx11Shader(L"dx11/color.fx", L"VS", L"vs_4_0", gm::GM_VERTEX_SHADER);
	GM_COM_CHECK(hr);
	hr = gm::GMLoadDx11Shader(L"dx11/color.fx", L"PS", L"ps_4_0", gm::GM_PIXEL_SHADER);
	GM_COM_CHECK(hr);
#endif
}