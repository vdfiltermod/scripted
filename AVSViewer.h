//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2001 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_AVSVIEWER_H
#define f_AVSVIEWER_H

#include <windows.h>

#define AVSEDITORCLASS (szAVSEditorClassName)
#define DEFAULT_EDITOR_FONT	"FixedSys"

#define REG_WINDOW_MAIN "Main"
#define REG_WINDOW_FIND "Find"

#ifndef f_AVSVIEWER_CPP
extern const char szAVSEditorClassName[];
#endif

const int SCRIPTTYPE_NONE = 0;
const int SCRIPTTYPE_AVS = 1;
const int SCRIPTTYPE_DECOMB = 2;
const int SCRIPTTYPE_VPS = 3;
const int SCRIPTTYPE_MAX = SCRIPTTYPE_VPS;

const char *const scripttypeName[SCRIPTTYPE_MAX+1] = {"None", "AviSynth", "Decomb", "VapourSynth"};

extern HWND g_ScriptEditor;

bool IsScriptType(const wchar_t *fn, int type);
int GetScriptType(const wchar_t *fn);


void InitAVSEditor();
void DeinitAVSEditor();

ATOM RegisterAVSEditor();

HWND AVSEdit(HWND, HWND, bool);
bool HandleFilename(HWND hwnd, const wchar_t* path);
bool ProcessDialogs(MSG& msg);
bool ProcessHotkeys(MSG& msg);
void HandleError(const char* s, void* userData);
void AVSViewerLoadSettings(HWND hwnd, const char* name);
void AVSViewerSaveSettings(HWND hwnd, const char* name);
void UpdatePreferences();

#endif
