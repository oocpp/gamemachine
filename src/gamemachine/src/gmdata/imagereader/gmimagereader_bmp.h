﻿#ifndef __IMAGEREADER_BMP_H__
#define __IMAGEREADER_BMP_H__
#include <gmcommon.h>
#include <string>
#include "gmimagereader.h"
BEGIN_NS

class GMImage;
struct BitmapFile;
class GMImageReader_BMP : public IImageReader
{
public:
	virtual bool load(const GMbyte* byte, GMsize_t size, OUT GMImage** img) override;
	virtual bool test(const GMbyte* byte) override;

private:
	void writeDataToImage(BitmapFile& bitmap, GMImage* img, GMuint size);
	void flipVertically(GMbyte* data, GMuint width, GMuint height);
};

END_NS
#endif