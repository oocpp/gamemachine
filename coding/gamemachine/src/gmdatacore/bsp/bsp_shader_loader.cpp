﻿#include "stdafx.h"
#include "bsp_shader_loader.h"
#include <vector>
#include <string>
#include "utilities/path.h"
#include "utilities/tinyxml/tinyxml.h"
#include "gmdatacore/shader.h"
#include "gmengine/elements/bspgameworld.h"
#include "utilities/scanner.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "gmengine/controllers/factory.h"
#include "gmengine/controllers/gamemachine.h"

#define BEGIN_PARSE(name) if ( strEqual(it->Value(), #name) ) parse_##name(shader, it)
#define BEGIN_PARSE_I(name, i) if ( strEqual(it->Value(), #name) ) parse_##name(shader, it, i)
#define PARSE(name) else if ( strEqual(it->Value(), #name) ) parse_##name(shader, it)
#define END_PARSE

enum
{
	GMS_SURFACE_FLAG_MAX = 19
};

struct SurfaceFlags
{
	const char* name;
	GMuint flag;
} _surface_flags[GMS_SURFACE_FLAG_MAX] = {
	{ "nodamage", SURF_NODAMAGE },
	{ "slick", SURF_SLICK },
	{ "sky", SURF_SKY },
	{ "ladder", SURF_LADDER },
	{ "noimpact", SURF_NOIMPACT },
	{ "nomarks", SURF_NOMARKS },
	{ "flesh", SURF_FLESH },
	{ "nodraw", SURF_NODRAW },
	{ "hint", SURF_HINT },
	{ "skip", SURF_SKIP },
	{ "nolightmap", SURF_NOLIGHTMAP },
	{ "pointlight", SURF_POINTLIGHT },
	{ "metalsteps", SURF_METALSTEPS },
	{ "nosteps", SURF_NOSTEPS },
	{ "nonsolid", SURF_NONSOLID },
	{ "lightfilter", SURF_LIGHTFILTER },
	{ "alphashadow", SURF_ALPHASHADOW },
	{ "nodlight", SURF_NODLIGHT },
	{ "dust", SURF_DUST },
};

static GMuint parseSurfaceParm(const char* p)
{
	for (GMuint i = 0; i < GMS_SURFACE_FLAG_MAX; i++)
	{
		if (strEqual(p, _surface_flags[i].name))
			return _surface_flags[i].flag;
	}

	ASSERT(false);
	gm_error("wrong surfaceparm %s", p);
	return 0;
}

static GMS_BlendFunc parseBlendFunc(const char* p)
{
	if (strEqual(p, "GMS_ONE"))
		return GMS_ONE;

	return GMS_ZERO;
}

static void parseBlendFunc(TextureFrames* frame, TiXmlElement* elem)
{
	const char* b = elem->Attribute("blendFunc");
	if (b)
	{
		Scanner s(b);
		char blendFunc[LINE_MAX];
		s.next(blendFunc);
		frame->blendFactors[0] = parseBlendFunc(blendFunc);
		s.next(blendFunc);
		frame->blendFactors[1] = parseBlendFunc(blendFunc);
		frame->blend = true;
	}
	else
	{
		frame->blend = false;
	}
}

static void loadImage(const char* filename, OUT Image** image)
{
	ImageReader imgReader;
	if (imgReader.load(filename, image))
		gm_info("loaded texture %s from shader", filename);
	else
		gm_error("texture %s not found", filename);
}

BSPShaderLoader::BSPShaderLoader(const char* directory, BSPGameWorld& world)
	: m_world(world)
	, m_directory(directory)
{
}

ITexture* BSPShaderLoader::addTextureToWorld(Shader& shader, const char* name)
{
	ResourceContainer* rc = m_world.getGraphicEngine()->getResourceContainer();
	TextureContainer& tc = rc->getTextureContainer();
	const TextureContainer::TextureItem* item = tc.find(name);
	if (!item)
	{
		std::string fn = m_world.bspWorkingDirectory();
		fn.append(name);
		Image* img = nullptr;
		loadImage(fn.c_str(), &img);

		if (img)
		{
			ITexture* texture;
			IFactory* factory = m_world.getGameMachine()->getFactory();
			factory->createTexture(img, &texture);

			TextureContainer::TextureItem ti;
			ti.name = name;
			ti.texture = texture;
			tc.insert(ti);
			return texture;
		}
		return nullptr;
	}
	else
	{
		return item->texture;
	}
}

void BSPShaderLoader::parseAll()
{
	std::vector<std::string> files = Path::getAllFiles(m_directory.c_str());
	for (auto iter = files.begin(); iter != files.end(); iter++)
	{
		parse((*iter).c_str());
	}
}

void BSPShaderLoader::parse(const char* filename)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(filename))
	{
		gm_error("xml load error at %d: %s", doc.ErrorRow(), doc.ErrorDesc());
	}
	TiXmlElement* elem = doc.RootElement();
	TiXmlElement* it = elem->FirstChildElement();
	for (; it; it = it->NextSiblingElement())
	{
		parseItem(it);
	}
}

void BSPShaderLoader::parseItem(TiXmlElement* elem)
{
	if (!strEqual(elem->Value(), "item"))
		gm_warning("First node must be 'item'.");

	m_textureNum = 0;
	Shader shader;
	const char* name = elem->Attribute("name");

	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement())
	{
		BEGIN_PARSE(surfaceparm);
		PARSE(cull);
		PARSE(animMap, m_textureNum);
		PARSE(clampmap, m_textureNum);
		PARSE(map, m_textureNum);
		END_PARSE;
	}

	m_world.addShader(name, shader);
}

void BSPShaderLoader::parse_surfaceparm(Shader& shader, TiXmlElement* elem)
{
	shader.surfaceFlag |= parseSurfaceParm(elem->GetText());
}

void BSPShaderLoader::parse_cull(Shader& shader, TiXmlElement* elem)
{
	const char* text = elem->GetText();
	if (strEqual(text, "none"))
		shader.cull = GMS_NONE;
	else if (strEqual(text, "cull"))
		shader.cull = GMS_CULL;
	else
		gm_error("wrong cull param %s", text);
}

void BSPShaderLoader::parse_animMap(Shader& shader, TiXmlElement* elem)
{
	TextureFrames* frame = &shader.texture.textureFrames[m_textureNum];
	GMint ms;
	SAFE_SSCANF(elem->Attribute("ms"), "%d", &ms);
	frame->animationMs = ms;

	GMuint frameCount = 0;
	for (TiXmlElement* it = elem->FirstChildElement(); it; it = it->NextSiblingElement(), frameCount++)
	{
		parseBlendFunc(frame, elem);
		BEGIN_PARSE_I(src, frameCount);
		END_PARSE;
	}
	frame->frameCount = frameCount;
	m_textureNum++;
}

void BSPShaderLoader::parse_src(Shader& shader, TiXmlElement* elem, GMuint i)
{
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
		shader.textures[i].textures[TEXTURE_INDEX_AMBIENT + m_textureNum] = texture;
}

void BSPShaderLoader::parse_clampmap(Shader& shader, TiXmlElement* elem)
{
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
	{
		parseBlendFunc(shader.textures[0], elem);
		shader.textures[0].wrapS = GMS_MIRRORED_REPEAT;
		shader.textures[0].wrapT = GMS_MIRRORED_REPEAT;
		shader.textures[0].textures[TEXTURE_INDEX_AMBIENT + m_textureNum] = texture;
	}

	// TODO
	shader.frameCount = 1;
	m_textureNum++;
}

void BSPShaderLoader::parse_map(Shader& shader, TiXmlElement* elem)
{
	ITexture* texture = addTextureToWorld(shader, elem->GetText());
	if (texture)
	{
		parseBlendFunc(shader.textures[0], elem);
		shader.textures[0].wrapS = GMS_REPEAT;
		shader.textures[0].wrapT = GMS_REPEAT;
		shader.textures[0].textures[TEXTURE_INDEX_AMBIENT + m_textureNum] = texture;
	}

	shader.frameCount = 1;
	m_textureNum++;
}