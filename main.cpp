#include "stdafx.h"
#include <vd2/system/VDString.h>
#include <vd2/system/registry.h>
#include <vd2/plugin/vdtool.h>
#include <vd2/VDXFrame/Unknown.h>
#include "CAviSynth.h"
#include "AVSViewer.h"
#include "Tdll.h"
#include "accel.h"
#include "prefs.h"
#include "api.h"

HINSTANCE g_hInst;
ATOM		g_classAVS;
Tdll		*g_dllRichedit;
CAviSynth	*g_dllAviSynth;
HACCEL		g_hAccelAVS;
extern const ACCELKEYTABLE_AVS g_accelAVSDefault;
extern char g_accelKEYDescription[256][255];
extern char g_accelAVSDescription[VDM_ACCEL_AVS_COUNT][255];
extern WORD g_accelKEYKeycode[256];
extern WORD g_accelAVSCommand[VDM_ACCEL_AVS_COUNT];


extern "C" bool Scintilla_RegisterClasses(void *hInstance);
extern "C" bool Scintilla_ReleaseResources();

/*
int VDTextAToW(wchar_t *dst, int max_dst, const char *src, int max_src) {
	*dst = 0;

	int len = MultiByteToWideChar(CP_ACP, 0, src, max_src, dst, max_dst);

	// remove null terminator if source was null-terminated (source
	// length was provided)
	return max_src<0 && len>0 ? len-1 : len;
}

int VDTextWToA(char *dst, int max_dst, const wchar_t *src, int max_src) {
	*dst = 0;

	int len = WideCharToMultiByte(CP_ACP, 0, src, max_src, dst, max_dst, NULL, NULL);

	// remove null terminator if source was null-terminated (source
	// length was provided)
	return max_src<0 && len>0 ? len-1 : len;
}
*/

void VDUISaveWindowPlacementW32(HWND hwnd, const char *name) {
	VDRegistryKey key(REG_KEY_APP"\\Window Placement");

	WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};

	if (GetWindowPlacement(hwnd, &wp))
		key.setBinary(name, (const char *)&wp.rcNormalPosition, sizeof(RECT));
}

void VDUIRestoreWindowPlacementW32(HWND hwnd, const char *name) {
	if (!IsZoomed(hwnd) && !IsIconic(hwnd)) {
		VDRegistryKey key(REG_KEY_APP"\\Window Placement");
		RECT r;

		if (key.getBinaryLength(name) == sizeof(r) && key.getBinary(name, (char *)&r, sizeof r)) {
			WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};

			if (GetWindowPlacement(hwnd, &wp)) {
				wp.length			= sizeof(WINDOWPLACEMENT);
				wp.flags			= 0;
				wp.showCmd			= SW_SHOWNORMAL;
				wp.rcNormalPosition	= r;

				SetWindowPlacement(hwnd, &wp);
			}
		}
	}
}

void AVSViewerLoadSettings(HWND hwnd, const char* name) {
	if (!hwnd) return;
	VDUIRestoreWindowPlacementW32(hwnd, name);
}

void AVSViewerSaveSettings(HWND hwnd, const char* name) {
	if (!hwnd) return;
	VDUISaveWindowPlacementW32(hwnd, name);
}

/*bool TranslateAcceleratorAvs(HWND hwnd, MSG& msg) {
	if (hwnd != g_hWnd)
		if (GetClassLong(hwnd, GCW_ATOM) == g_classAVS)
			if (TranslateAccelerator(hwnd, g_hAccelAVS, &msg)) return true;
	return false;
}*/

void VDRequestFrameSize(vd_framesize& frame)
{
	frame.frametype = 0;
	RECT r = {0,0,100,100};
	frame.frame = r;
}

bool initialize()
{
	LoadPrefs();
	InitDescriptions();
	g_classAVS = RegisterAVSEditor();
	InitAVSEditor();
	Scintilla_RegisterClasses(g_hInst);
	g_dllRichedit = new Tdll("Riched20.dll", NULL);
	if (!g_dllRichedit->ok) return false;
	g_dllAviSynth = new CAviSynth("avisynth.dll");
	g_hAccelAVS = CreateAVSAccelerators();
	return true;
}

void uninitialize()
{
	DeinitAVSEditor();
	Scintilla_ReleaseResources();
	delete g_dllAviSynth;
	delete g_dllRichedit;
	DestroyAcceleratorTable(g_hAccelAVS);
}

bool TranslateMessages(MSG& msg)
{
	if (ProcessHotkeys(msg)) return true;
	if (ProcessDialogs(msg)) return true;
	return false;
}

#ifndef PLUGIN

wchar_t g_fileName[MAX_PATH];

void VDGetFilename(wchar_t* buf, size_t n)
{
	wcscpy_s(buf,n,g_fileName);
}

void VDSetFilename(wchar_t* s, void*)
{
	wcscpy_s(g_fileName,MAX_PATH,s);
}

void VDSendReopen(void* userData)
{
	::HandleError("Reopen not implemented",userData);
}

int64 VDRequestPos()
{
	return 0;
}

void VDSetPos(int64 pos)
{
}

void VDRequestRange(int64& r0, int64& r1)
{
	r0 = 0;
	r1 = 10;
}

void VDRequestFrameset(vd_frameset& set, int max)
{
	set.count = 0;
	if(max<2) return;
	set.count = 2;
	set.ranges[0].from = 0;
	set.ranges[0].to = 10;
	set.ranges[1].from = 0;
	set.ranges[1].to = 10;
}

int APIENTRY WinMain(HINSTANCE hInstance,
										 HINSTANCE hPrevInstance,
										 LPSTR     lpCmdLine,
										 int       nCmdShow)
{
	::g_hInst = hInstance;

	if(__argc>1){
		char* s = __argv[1];
		VDTextAToW(g_fileName,MAX_PATH,s);
	}

	initialize();
	AVSEdit(NULL, NULL, true);

	while(true){
		MSG msg;
		while(PeekMessage(&msg,0,0,0,PM_NOREMOVE)){
			if(!GetMessage(&msg,0,0,0)){
				return 0;
			}
		
			if(TranslateMessages(msg)) continue;

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if (g_ScriptEditor==(HWND)-1) break;
	}

	uninitialize();

	return 0;
}

#endif

#ifdef PLUGIN

const VDXToolContext* g_context;

void VDGetFilename(wchar_t* buf, size_t n)
{
	g_context->mpCallbacks->GetFileName(buf,n);
}

void VDSetFilename(wchar_t* s, void* userData)
{
	g_context->mpCallbacks->SetFileName(s, L"AVIFile/Avisynth input driver (internal)", userData);
}

void VDSendReopen(void* userData)
{
	g_context->mpCallbacks->Reopen(userData);
}

int64 VDRequestPos()
{
	IVDTimeline* t = g_context->mpCallbacks->GetTimeline();
	return t->GetTimelinePos();
}

void VDSetPos(int64 pos)
{
	IVDTimeline* t = g_context->mpCallbacks->GetTimeline();
	t->SetTimelinePos(pos);
}

void VDRequestRange(int64& r0, int64& r1)
{
	IVDTimeline* t = g_context->mpCallbacks->GetTimeline();
	int64 start;
	int64 end;
	t->GetSelection(start,end);
	r0 = start;
	r1 = end;
}

void VDRequestFrameset(vd_frameset& set, int max)
{
	IVDTimeline* t = g_context->mpCallbacks->GetTimeline();
	int count = t->GetSubsetCount();
	set.count = 0;
	if(max<count) return;
	set.count = count;
	for(int i=0; i<count; i++){
		int64 start;
		int64 end;
		t->GetSubsetRange(i,start,end);
		set.ranges[i].from = start;
		set.ranges[i].to = end;
	}
}

class ToolDriver: public vdxunknown<IVDXTool> {
	virtual bool VDXAPIENTRY GetMenuInfo(int id, char* name, int name_size, bool* enabled) {
		if (id==0) {
			strcpy(name,"Script Editor");
			*enabled = true;
			return true;
		}
		return false;
	}
	virtual bool VDXAPIENTRY GetCommandId(int id, char* name, int name_size) {
		if (id==0) {
			strcpy(name,"Tools.ScriptEditor");
			return true;
		}
		return false;
	}
	virtual bool VDXAPIENTRY ExecuteMenu(int id, VDXHWND hwndParent) {
		if (id==0) {
			AVSEdit(NULL, (HWND)hwndParent, true);
			return true;
		}
		return false;
	}
	virtual bool VDXAPIENTRY TranslateMessage(MSG& msg) {
		return TranslateMessages(msg);
	}
	virtual bool VDXAPIENTRY HandleError(const char* s, int source, void* userData) {
		::HandleError(s,userData);
		return true;
	}
	virtual bool VDXAPIENTRY HandleFileOpen(const wchar_t* fileName, const wchar_t* driverName, VDXHWND hwndParent) {
		return HandleFilename((HWND)hwndParent,fileName);
	}
};

bool VDXAPIENTRY create(const VDXToolContext *pContext, IVDXTool **pp)
{
	ToolDriver *p = new ToolDriver();
	if(!p) return false;
	*pp = p;
	p->AddRef();
	g_context = pContext;
	return true;
}

bool VDXAPIENTRY config(VDXHWND parent)
{
	ShowPrefs((HWND)parent);
	return true;
}

VDXToolDefinition tool_def={
	sizeof(VDXToolDefinition),
	create
};

VDXPluginInfo plugin_def={
	sizeof(VDXPluginInfo),
	L"Script Editor",
	L"Anton Shekhovtsov",
	L"Text editor for avs scripts.",
	1,
	kVDXPluginType_Tool,
	0,
	kVDXPlugin_APIVersion,
	kVDXPlugin_APIVersion,
	kVDXPlugin_ToolAPIVersion,
	kVDXPlugin_ToolAPIVersion,
	&tool_def,
	0,
	config
};

VDPluginInfo* kPlugins[]={&plugin_def,0};

extern "C" VDPluginInfo** VDXAPIENTRY VDGetPluginInfo()
{
	return kPlugins;
}

BOOLEAN WINAPI DllMain( IN HINSTANCE hDllHandle, IN DWORD nReason, IN LPVOID Reserved )
{
	switch ( nReason ){
	case DLL_PROCESS_ATTACH:
		g_hInst = hDllHandle;
		initialize();
		return true;

	case DLL_PROCESS_DETACH:
		if(Reserved==0) uninitialize();
		return true;
	}

	return true;
}

#endif
