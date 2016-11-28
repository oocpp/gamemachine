﻿#define BEGIN_NS namespace fglextlib {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	className##Private& dataRef() { return m_data; }

#ifndef _WINDOWS
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned char BYTE;
#endif



// Options
#include "gl/GL.h"
typedef unsigned char FByte;
typedef GLfloat Ffloat;
typedef GLint Fint;
#define LINE_MAX 256