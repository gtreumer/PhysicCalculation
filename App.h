#pragma once
#ifndef __APP_H
#define __APP_H

#include "windows.h"
#include "Thread.h"

namespace Test
{
	class TextCaption;
	class App
	{
	public:
		App() {}
		App(HINSTANCE hInstance, int nCmdShow, int width, int height);
		virtual ~App();

		void start();
		void render();
		void update();
		void init();

		LRESULT CALLBACK WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK ThisWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		HWND createWindow(HINSTANCE hInstance, int nCmdShow, int width, int height);
	private:
		static LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		HINSTANCE m_hInstance;
		int m_nCmdShow;
		int m_width;
		int m_height;
		HWND mHwnd;
		HWND mButton;
		HWND mVelEditBox;
		HWND mAngleEditBox;
		Sys::Thread* mThread;
		Sys::Thread::Runnable* mProcessRunnable;
		TextCaption* mVelocityCaption;
		TextCaption* mAngleCaption;
	};
}

#endif