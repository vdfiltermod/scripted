#include "stdafx.h"
#include <vd2/system/VDString.h>
#include <vd2/system/registry.h>
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

wchar_t g_fileName[MAX_PATH];

void VDGetFilename(wchar_t* buf, size_t n)
{
  wcscpy_s(buf,n,g_fileName);
}

void VDSetFilename(wchar_t* s)
{
  wcscpy_s(g_fileName,MAX_PATH,s);
}

int64 VDRequestPos()
{
  return 0;
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

void VDRequestFrameSize(vd_framesize& frame)
{
  frame.frametype = 0;
  RECT r = {0,0,100,100};
  frame.frame = r;
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

  LoadPrefs();
	InitDescriptions();
	g_classAVS = RegisterAVSEditor();
	InitAVSEditor();
  Scintilla_RegisterClasses(g_hInst);
	g_dllRichedit = new Tdll("Riched20.dll", NULL);
	if (!g_dllRichedit->ok) return false;
	g_dllAviSynth = new CAviSynth("avisynth.dll");
  g_hAccelAVS = CreateAVSAccelerators();

	AVSEdit(NULL, NULL, true);

  while(true){
    MSG msg;
    while(PeekMessage(&msg,0,0,0,PM_NOREMOVE)){
      if(!GetMessage(&msg,0,0,0)){
        return 0;
      }
    
    	HWND hwnd = GetAncestor(msg.hwnd, GA_ROOT);
			if (TranslateAccelerator(hwnd, g_hAccelAVS, &msg)) continue;
      if (ProcessDialogs(msg)) continue;

      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }

    if (g_ScriptEditor==(HWND)-1) break;
  }

	DeinitAVSEditor();
  Scintilla_ReleaseResources();
	delete g_dllAviSynth;
	delete g_dllRichedit;
  DestroyAcceleratorTable(g_hAccelAVS);

  return 0;
}
