﻿#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "gmtypoengine.h"

enum Margins
{
	Left = 0,
	Top,
	Right,
	Bottom,
};

//GlyphObject
template <typename T>
inline bool isValidRect(const T& r)
{
	return r.width >= 0;
}

#define BEGIN_GEOMETRY_TO_VIEWPORT(clientRect) const auto& cw = clientRect.width, &ch = clientRect.height;
#define GEOMETRY_TO_VIEWPORT_X(i) ((i) * 2.f / cw - 1.f)
#define GEOMETRY_TO_VIEWPORT_Y(i) (1 - (i) * 2.f / ch)
#define END_GEOMETRY_TO_VIEWPORT()
#define UV_INDEX(points) points[0], points[1], 0
#define EXTENTS_INDEX(points) points[0], points[1], 1

#define BEGIN_GLYPH_XY(resolutionWidth, resolutionHeight) { GMfloat __resolutionWidth = resolutionWidth, __resolutionHeight = resolutionHeight;
#define X(i) (i) * 2 / __resolutionWidth
#define Y(i) (i) * 2 / __resolutionHeight
#define END_GLYPH_XY() }

#define UV_X(i) ((i) / (GMfloat)GMGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGlyphManager::CANVAS_HEIGHT)

GMGlyphObject::GMGlyphObject()
{
	D(d);
	d->typoEngine = new GMTypoEngine();
}

GMGlyphObject::GMGlyphObject(ITypoEngine* typo)
{
	D(d);
	d->typoEngine = typo;
	d->insetTypoEngine = false;
}

GMGlyphObject::~GMGlyphObject()
{
	D(d);
	GMModel* m = getModel();
	GM_delete(m);
	if (d->insetTypoEngine)
		GM_delete(d->typoEngine);
}

void GMGlyphObject::setText(const GMWchar* text)
{
	D(d);
	d->text = text;
}

void GMGlyphObject::constructModel()
{
	D(d);
	GMModel* model = new GMModel();
	GMMesh* child = new GMMesh();
	model->append(child);
	child->setArrangementMode(GMArrangementMode::Triangle_Strip);
	child->setType(GMMeshType::Glyph);

	GMComponent* component = new GMComponent(child);
	onCreateShader(component->getShader());
	createVertices(component);
	child->appendComponent(component);

	GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, model);
	setModel(asset);
}

void GMGlyphObject::onCreateShader(Shader& shader)
{
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0).addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NONE);
	shader.setBlend(true);
	shader.setBlendFactorSource(GMS_BlendFunc::SRC_ALPHA);
	shader.setBlendFactorDest(GMS_BlendFunc::ONE);
}

void GMGlyphObject::updateModel()
{
	D(d);
	GMComponent* component = getModel()->getAllMeshes()[0]->getComponents()[0];
	component->clear();
	createVertices(component);
	GMModelPainter* painter = getModel()->getPainter();
	painter->dispose();
	painter->transfer();
}

void GMGlyphObject::createVertices(GMComponent* component)
{
	D(d);

	D_BASE(db, GMControlGameObject);
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	IWindow* window = GM.getMainWindow();

	constexpr GMfloat Z = 0;
	GMRect rect = d->autoResize && isValidRect(d->lastClientRect) ? d->lastClientRect : GM.getMainWindow()->getClientRect();
	GMRectF coord = d->autoResize && isValidRect(d->lastGeometry) ? d->lastGeometry : toViewportCoord(db->geometry);

	if (!isValidRect(d->lastClientRect) || d->autoResize)
	{
		GM_ASSERT(!isValidRect(d->lastGeometry));
		d->lastClientRect = rect;
		d->lastGeometry = coord;
	}

	BEGIN_GLYPH_XY(rect.width, rect.height)
		// 使用排版引擎进行排版
		ITypoEngine* typoEngine = d->typoEngine;
		GMTypoOptions options;
		options.typoArea.width = coord.width * rect.width;
		options.typoArea.height = coord.width * rect.height;

		GM_ASSERT(typoEngine);
		GMTypoIterator iter = typoEngine->begin(d->text, options);
		for (; iter != typoEngine->end(); ++iter)
		{
			const GMTypoResult& typoResult = *iter;
			const GMGlyphInfo& glyph = *typoResult.glyph;

			component->beginFace();
			if (glyph.width > 0 && glyph.height > 0)
			{
				// 如果width和height为0，视为空格，只占用空间而已
				// 否则：按照条带顺序，创建顶点
				// 0 2
				// 1 3
				// 让所有字体origin开始的x轴平齐

				component->vertex(coord.x + X(typoResult.x), coord.y - Y(typoResult.y + typoResult.lineHeight - glyph.bearingY), Z);
				component->vertex(coord.x + X(typoResult.x), coord.y - Y(typoResult.y + typoResult.lineHeight - (glyph.bearingY - glyph.height)), Z);
				component->vertex(coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + typoResult.lineHeight - glyph.bearingY), Z);
				component->vertex(coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + typoResult.lineHeight - (glyph.bearingY - glyph.height)), Z);

				component->uv(UV_X(glyph.x), UV_Y(glyph.y));
				component->uv(UV_X(glyph.x), UV_Y(glyph.y + glyph.height));
				component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y));
				component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height));

				component->color(typoResult.color[0], typoResult.color[1], typoResult.color[2]);
				component->color(typoResult.color[0], typoResult.color[1], typoResult.color[2]);
				component->color(typoResult.color[0], typoResult.color[1], typoResult.color[2]);
				component->color(typoResult.color[0], typoResult.color[1], typoResult.color[2]);
			}

			component->endFace();
		}
	END_GLYPH_XY()
}

void GMGlyphObject::onAppendingObjectToWorld()
{
	D(d);
	d->lastRenderText = d->text;
	constructModel();
}

void GMGlyphObject::draw()
{
	D(d);
	if (d->lastRenderText != d->text)
	{
		update();
		d->lastRenderText = d->text;
	}
	GMGameObject::draw();
}

void GMGlyphObject::update()
{
	D_BASE(d, GMGameObject);
	updateModel();
}

//////////////////////////////////////////////////////////////////////////
GMImage2DBorder::GMImage2DBorder(GMAsset texture,
	const GMRect& borderTextureGeometry,
	GMfloat textureWidth,
	GMfloat textureHeight,
	GMfloat cornerWidth,
	GMfloat cornerHeight
)
{
	D(d);
	d->texture = texture;
	d->borderTextureGeometry = borderTextureGeometry;
	d->width = textureWidth;
	d->height = textureHeight;
	d->cornerWidth = cornerWidth;
	d->cornerHeight = cornerHeight;
}

GMImage2DBorder::~GMImage2DBorder()
{
	D(d);
	release(d->models);
	release(d->objects);
}

template <typename T, GMint Size> void GMImage2DBorder::release(T* (&m)[Size])
{
	for (GMint i = 0; i < Size; ++i)
	{
		GM_delete(m[i]);
	}
}

void GMImage2DBorder::clone(GMImage2DBorder& b)
{
	D(d);
	D_OF(d_b, &b);
	d->texture = d_b->texture;
	d->borderTextureGeometry = d_b->borderTextureGeometry;
	d->width = d_b->width;
	d->height = d_b->height;
	d->cornerWidth = d_b->cornerWidth;
	d->cornerHeight = d_b->cornerHeight;
}

void GMImage2DBorder::createBorder(const GMRect& geometry)
{
	D(d);
	GM_ASSERT(hasBorder());

	// 创建9个区域
	// 0(corner) 3(center) 6(corner)
	// 1(middle)     4     7(middle)
	// 2(corner) 5(center) 8(corner)

	const GMfloat& textureWidth = d->width,
		&textureHeight = d->height;

	const GMRect& textureGeo = textureGeometry();
	GMfloat base[2] = {
		textureGeo.x / (GMfloat)textureWidth,
		textureGeo.y / (GMfloat)textureHeight,
	};

	const GMfloat& center_w_pixel = (textureGeo.width - 2 * d->cornerWidth), &center_h_pixel = d->cornerHeight;
	const GMfloat& middle_w_pixel = d->cornerWidth, &middle_h_pixel = (textureGeo.height - 2 * d->cornerHeight);
	const GMfloat& corner_w = d->cornerWidth / d->width, &corner_h = d->cornerHeight / d->height;
	const GMfloat& corner_w2 = corner_w * 2, &corner_h2 = corner_h * 2;
	const GMfloat& center_w = center_w_pixel / d->width, &center_h = corner_h;
	const GMfloat& middle_w = corner_w, &middle_h = middle_h_pixel / d->height;

	const GMfloat uv_points[16][2] = {
		{ base[0], base[1] + corner_h2 + middle_h },
		{ base[0], base[1] + corner_h + middle_h },
		{ base[0], base[1] + corner_h },
		{ base[0], base[1] },

		{ base[0] + corner_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w, base[1] + corner_h },
		{ base[0] + corner_w, base[1] },

		{ base[0] + corner_w + center_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w + center_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w + center_w, base[1] + corner_h },
		{ base[0] + corner_w + center_w, base[1] },

		{ base[0] + corner_w2 + center_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w2 + center_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w2 + center_w, base[1] + corner_h },
		{ base[0] + corner_w2 + center_w, base[1] },
	};

	GMfloat uv[9][12] = {
		UV_INDEX(uv_points[0]), UV_INDEX(uv_points[1]), UV_INDEX(uv_points[5]), UV_INDEX(uv_points[4]),
		UV_INDEX(uv_points[1]), UV_INDEX(uv_points[2]), UV_INDEX(uv_points[6]), UV_INDEX(uv_points[5]),
		UV_INDEX(uv_points[2]), UV_INDEX(uv_points[3]), UV_INDEX(uv_points[7]), UV_INDEX(uv_points[6]),
		UV_INDEX(uv_points[4]), UV_INDEX(uv_points[5]), UV_INDEX(uv_points[9]), UV_INDEX(uv_points[8]),
		UV_INDEX(uv_points[5]), UV_INDEX(uv_points[6]), UV_INDEX(uv_points[10]), UV_INDEX(uv_points[9]),
		UV_INDEX(uv_points[6]), UV_INDEX(uv_points[7]), UV_INDEX(uv_points[11]), UV_INDEX(uv_points[10]),
		UV_INDEX(uv_points[8]), UV_INDEX(uv_points[9]), UV_INDEX(uv_points[13]), UV_INDEX(uv_points[12]),
		UV_INDEX(uv_points[9]), UV_INDEX(uv_points[10]), UV_INDEX(uv_points[14]), UV_INDEX(uv_points[13]),
		UV_INDEX(uv_points[10]), UV_INDEX(uv_points[11]), UV_INDEX(uv_points[15]), UV_INDEX(uv_points[14]),
	};

	GMRect w = GM.getMainWindow()->getClientRect();
	GMRectF window = {
		(GMfloat)w.x,
		(GMfloat)w.y,
		(GMfloat)w.width,
		(GMfloat)w.height
	};

	const GMfloat &pos_center_w = geometry.width - 2 * d->cornerWidth,
		&pos_center_h = d->cornerHeight,
		&pos_middle_w = d->cornerWidth,
		&pos_middle_h = geometry.height - 2 * d->cornerHeight;


	GMfloat extentsArray[4][3] = {
		{ d->cornerWidth / window.width, d->cornerHeight / window.height, 1 },
		{ pos_middle_w / window.width, pos_middle_h / window.height , 1 },
		{ pos_center_w / window.width, center_h_pixel / window.height, 1 },
		{ pos_center_w / window.width, pos_middle_h / window.height, 1 },
	};

	GMfloat extents[9][3] = {
		EXTENTS_INDEX(extentsArray[0]),
		EXTENTS_INDEX(extentsArray[1]),
		EXTENTS_INDEX(extentsArray[0]),

		EXTENTS_INDEX(extentsArray[2]),
		EXTENTS_INDEX(extentsArray[3]),
		EXTENTS_INDEX(extentsArray[2]),

		EXTENTS_INDEX(extentsArray[0]),
		EXTENTS_INDEX(extentsArray[1]),
		EXTENTS_INDEX(extentsArray[0]),
	};

	BEGIN_GEOMETRY_TO_VIEWPORT(window)
		GMfloat pos[9][3] = {
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x), GEOMETRY_TO_VIEWPORT_Y(geometry.y), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight + pos_middle_h), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth), GEOMETRY_TO_VIEWPORT_Y(geometry.y), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight + pos_middle_h), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth + pos_center_w), GEOMETRY_TO_VIEWPORT_Y(geometry.y), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth + pos_center_w), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight), 0 },
			{ GEOMETRY_TO_VIEWPORT_X(geometry.x + d->cornerWidth + pos_center_w), GEOMETRY_TO_VIEWPORT_Y(geometry.y + d->cornerHeight + pos_middle_h), 0 },
		};
	END_GEOMETRY_TO_VIEWPORT()

	// Shader回调
	struct _Callback: public IPrimitiveCreatorShaderCallback {
		_Callback(GMAsset t)
			: texture(t)
		{
		}

		void onCreateShader(Shader& shader)
		{
			shader.setNoDepthTest(true);
			auto& tex = shader.getTexture();
			auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
			frames.addFrame(GMAssets::getTexture(texture));
		}

		GMAsset texture;
	} _cb(d->texture);

	// 制作9个矩形进行拉伸（可以以后还会有非拉伸的模式）
	for (GMint i = 0; i < GM_dimensions_of_array(d->models); ++i)
	{
		GMPrimitiveCreator::createQuad(extents[i], pos[i], d->models + i, &_cb, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft, &uv[i]);

		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, *(d->models + i));
		d->objects[i] = new GMGameObject(asset);
		d->objects[i]->onAppendingObjectToWorld();
		GM.initObjectPainter(d->objects[i]->getModel());
	}
}

template <GMint Size> void GMImage2DBorder::drawObjects(GMGameObject* (&objects)[Size])
{
	IGraphicEngine* engine = GM.getGraphicEngine();
	for (GMint i = 0; i < Size; ++i)
	{
		objects[i]->draw();
	}
}

void GMImage2DBorder::draw()
{
	D(d);
	GM_ASSERT(hasBorder());
	drawObjects(d->objects);
}

//////////////////////////////////////////////////////////////////////////
GMImage2DGameObject::~GMImage2DGameObject()
{
	D(d);
	GMModel* model = getModel();
	if (model)
		delete model;
	if (d->textModel)
		delete d->textModel;
}

void GMImage2DGameObject::setPaddings(GMint left, GMint top, GMint right, GMint bottom)
{
	D(d);
	d->paddings[Left] = left;
	d->paddings[Top] = top;
	d->paddings[Right] = right;
	d->paddings[Bottom] = bottom;
}

void GMImage2DGameObject::setImage(GMAsset& image)
{
	D(d);
	d->image = GMAssets::getTexture(image);
	GM_ASSERT(d->image);
}

void GMImage2DGameObject::setText(const GMString& text)
{
	D(d);
	d->text = text.toStdWString();
	if (!d->text.empty())
		d->textModel = new GMGlyphObject();
}

void GMImage2DGameObject::setBorder(GMImage2DBorder& border)
{
	D(d);
	d->border.clone(border);
}

void GMImage2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	Base::onAppendingObjectToWorld();
	createBackgroundImage();
	createBorder();
	createGlyphs();
}

void GMImage2DGameObject::draw()
{
	D(d);

	// 首先创建出一个裁剪框
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginCreateStencil();
	Base::draw();
	engine->endCreateStencil();

	engine->beginUseStencil(false);

	// 背景
	Base::draw();

	// 边框
	if (d->border.hasBorder())
		d->border.draw();

	// 文字
	if (d->textModel)
		d->textModel->draw();

	engine->endUseStencil();
}

void GMImage2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setBlend(false);
	shader.setNoDepthTest(true);

	if (d->image)
	{
		auto& tex = shader.getTexture();
		auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
		frames.addFrame(d->image);
	}
}

void GMImage2DGameObject::createBackgroundImage()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	
	GMRectF coord = toViewportCoord(db->geometry);
	GMfloat extents[3] = {
		coord.width,
		coord.height,
		1.f,
	};
	GMModel* model = nullptr;
	GMfloat pos[3] = { coord.x, coord.y, 0 };
	GMPrimitiveCreator::createQuad(extents, pos, &model, this, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft);

	setModel(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
}

void GMImage2DGameObject::createBorder()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	if (d->border.hasBorder())
		d->border.createBorder(db->geometry);
}

void GMImage2DGameObject::createGlyphs()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	if (d->textModel)
	{
		GMRect geometry = {
			db->geometry.x + d->paddings[Left],
			db->geometry.y + d->paddings[Top],
			db->geometry.width - d->paddings[Left] - d->paddings[Right],
			db->geometry.height - d->paddings[Top] - d->paddings[Bottom],
		};

		d->textModel->setGeometry(geometry);
		d->textModel->setWorld(getWorld());
		d->textModel->setText(d->text.c_str());
		d->textModel->onAppendingObjectToWorld();
		GM.initObjectPainter(d->textModel->getModel());
		GMAssets::createIsolatedAsset(GMAssetType::Model, d->textModel);
	}
}

//////////////////////////////////////////////////////////////////////////
GMImage2DGameObject* GMListbox2DGameObject::addItem(const GMString& text)
{
	D(d);
	GMImage2DGameObject* item = new GMImage2DGameObject(this);
	item->setText(text);

	GMRect geo = getGeometry();
	geo.x = geo.y = 0;
	item->setGeometry(geo);
	return item;
}

void GMListbox2DGameObject::setItemMargins(GMint left, GMint top, GMint right, GMint bottom)
{
	D(d);
	d->itemMargins[Left] = left;
	d->itemMargins[Top] = top;
	d->itemMargins[Right] = right;
	d->itemMargins[Bottom] = bottom;
}

void GMListbox2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setNoDepthTest(true);
}

void GMListbox2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);

	Base::onAppendingObjectToWorld();
	auto x = db->geometry.x + d->itemMargins[Left],
		y = db->geometry.y + d->itemMargins[Top];

	for (auto item : getItems())
	{
		static GMRect rect;
		rect.x = x;
		rect.y = y;
		rect.width = item->getGeometry().width - d->itemMargins[Left] - d->itemMargins[Right];
		rect.height = item->getGeometry().height;
		item->setGeometry(rect);
		item->setWorld(getWorld());
		item->onAppendingObjectToWorld();
		GM.initObjectPainter(item->getModel());

		y += rect.height + d->itemMargins[Top] + d->itemMargins[Bottom];
	}
}

void GMListbox2DGameObject::draw()
{
	D(d);
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginCreateStencil();
	Base::draw();
	engine->endCreateStencil();

	engine->beginUseStencil(false);
	for (auto item : getItems())
	{
		item->draw();
	}
	engine->endUseStencil();
}

void GMListbox2DGameObject::notifyControl()
{
	D(d);
	Base::notifyControl();
	for (auto item : getItems())
	{
		item->notifyControl();
	}
}