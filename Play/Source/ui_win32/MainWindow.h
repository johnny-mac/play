#pragma once

#include <boost/signals2.hpp>
#include <string>
#include <memory>
#include "win32/Window.h"
#include "win32/StatusBar.h"
#include "SettingsDialogProvider.h"
#include "OutputWnd.h"
#include "AviStream.h"
#include "VirtualPad/VirtualPadWindow.h"
#include "StatsOverlayWindow.h"
#include "FutureContinuationManager.h"
#ifdef DEBUGGER_INCLUDED
#include "Debugger.h"
#include "FrameDebugger/FrameDebugger.h"
#endif
#include "../Profiler.h"
#include "../PS2VM.h"

class CMainWindow : public Framework::Win32::CWindow, public boost::signals2::trackable
{
public:
	CMainWindow(CPS2VM&);
	~CMainWindow();
	int Loop();

protected:
	long OnTimer(WPARAM) override;
	long OnCommand(unsigned short, unsigned short, HWND) override;
	long OnActivateApp(bool, unsigned long) override;
	long OnSize(unsigned int, unsigned int, unsigned int) override;
	long OnMove(int, int) override;
	long OnKeyDown(WPARAM, LPARAM) override;

private:
	class COpenCommand
	{
	public:
		virtual ~COpenCommand() = default;
		virtual void Execute(CMainWindow*) = 0;
	};

	class CBootCdRomOpenCommand : public COpenCommand
	{
	public:
		void Execute(CMainWindow*) override;
	};

	class CLoadElfOpenCommand : public COpenCommand
	{
	public:
		CLoadElfOpenCommand(const boost::filesystem::path&);
		void Execute(CMainWindow*) override;

	private:
		boost::filesystem::path m_executablePath;
	};

	typedef std::shared_ptr<COpenCommand> OpenCommandPtr;

	void OpenELF();
	void BootCDROM();
	void BootDiskImage();
	void RecordAvi();
	void ResumePause();
	void Reset();
	void PauseWhenFocusLost();
	void SaveState();
	void LoadState();
	void ChangeStateSlot(unsigned int);
	void ChangeViewMode(CGSHandler::PRESENTATION_MODE);
	void ToggleFullscreen();
	void ShowDebugger();
	void ShowFrameDebugger();
	void DumpNextFrame();
	void ToggleGsDraw();
	void ShowSysInfo();
	void ShowAbout();
	void ShowSettingsDialog(CSettingsDialogProvider*);
	void ShowVideoSettings();
	void ShowControllerSettings();
	void ShowVfsManager();
	void ShowMcManager();
	void ToggleSoundEnabled();

	void ProcessCommandLine();

	void LoadELF(const boost::filesystem::path&);
	void RefreshLayout();
	void RefreshOverlaysLayout();
	void PrintVersion(TCHAR*, size_t);
	void PrintStatusTextA(const char*, ...);
	void SetStatusText(const TCHAR*);
	void CreateAccelerators();

	void CreateDebugMenu();
	static boost::filesystem::path GetFrameDumpDirectoryPath();

	void CreateStateSlotMenu();
	void UpdateUI();

	void OnNewFrame(uint32);

	void OnExecutableChange();

	void SetupSoundHandler();

	void ScreenCapture();

	CPS2VM& m_virtualMachine;

	unsigned int m_frames;
	uint32 m_drawCallCount;
	HACCEL m_accTable;

	unsigned int m_stateSlot;

	bool m_pauseFocusLost;
	bool m_deactivatePause;

	OpenCommandPtr m_lastOpenCommand;

	CAviStream m_aviStream;
	bool m_recordingAvi;
	uint8* m_recordBuffer;
	HANDLE m_recordAviMutex;
	unsigned int m_recordBufferWidth;
	unsigned int m_recordBufferHeight;

	bool m_isFullscreen = false;
	WINDOWPLACEMENT m_windowPlacement;
	Framework::Win32::CStatusBar m_statusBar;
	COutputWnd* m_outputWnd;
	CVirtualPadWindow m_virtualPadWnd;
	CStatsOverlayWindow m_statsOverlayWnd;
#ifdef DEBUGGER_INCLUDED
	std::unique_ptr<CDebugger> m_debugger;
	std::unique_ptr<CFrameDebugger> m_frameDebugger;
#endif

	CFutureContinuationManager m_futureContinuationManager;

	static double m_statusBarPanelWidths[2];
};
