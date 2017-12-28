﻿#undef __STRICT_ANSI__
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "stdafx.h"
#include "debug.h"
#include <cwchar>

inline void format_timeW(GMWchar* in)
{
#if _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	swprintf(in, L"%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

inline void format_timeA(char* in)
{
#if _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	sprintf(in, "%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

#define f_timeW(t) GMWchar t[LINE_MAX]; format_timeW(t);
#define f_timeA(t) char t[LINE_MAX]; format_timeA(t);

#define printW(format, tag) \
	D(d);															\
	GMWchar out[LINE_MAX];											\
	va_list ap;														\
	va_start(ap, format);											\
	if (d && d->debugger)											\
	{																\
		GMWchar buf[LINE_MAX];										\
		vswprintf(buf, format, ap);									\
		f_timeW(t);													\
		wsprintf(out, _L("[") _L(#tag) _L("]%s: %s"), t, buf);		\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		GMWchar buf[LINE_MAX];										\
		vswprintf(buf, format, ap);									\
		f_timeW(t);													\
		wprintf(out, _L("[") _L(#tag) _L("]%s: %s"), t, buf);		\
	}																\
	va_end(ap);

#define printA(format, tag) \
	D(d);															\
	char out[LINE_MAX];												\
	va_list ap;														\
	va_start(ap, format);											\
	if (d && d->debugger)											\
	{																\
		char buf[LINE_MAX];											\
		vsprintf(buf, format, ap);									\
		f_timeA(t);													\
		sprintf(out, "[" #tag "]%s: %s", t, buf);					\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		char buf[LINE_MAX];											\
		vsprintf(buf, format, ap);									\
		f_timeA(t);													\
		printf(out, "[" #tag "]%s: %s", t, buf);					\
	}																\
	va_end(ap);

void GMDebugger::info(const GMWchar *format, ...)
{
	printW(format, info);
}

void GMDebugger::info(const char* format, ...)
{
	printA(format, info);
}

void GMDebugger::error(const GMWchar *format, ...)
{
	printW(format, error);
}

void GMDebugger::error(const char* format, ...)
{
	printA(format, error);
}

void GMDebugger::warning(const GMWchar *format, ...)
{
	printW(format, warning);
}

void GMDebugger::warning(const char* format, ...)
{
	printA(format, warning);
}

#ifdef _DEBUG
void GMDebugger::debug(const GMWchar *format, ...)
{
	printW(format, debug);
}

void GMDebugger::debug(const char* format, ...)
{
	printA(format, debug);
}
#endif

Map<size_t, void*> HookFactory::g_hooks;