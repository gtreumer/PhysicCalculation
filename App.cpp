#include "App.h"
#include "Timer.h"
#include "ConditionVariable.h"
#include "LockGuard.h"
#include "Time.h"
#include <cmath>

#define WM_START_ANIMATION (WM_USER + 0)
#define WM_STOP_ANIMATION  (WM_USER + 1)
#define WM_UPDATE_WND      (WM_USER + 2)
#define WM_RENDER_WND      (WM_USER + 3)
#define BUTTON_ID          1001

namespace Test
{
	template<typename T> 
	void SafeDeletePtr(T* ptr)
	{
		if (ptr)
		{
			delete ptr;
			ptr = NULL;
		}
	}

	int getTextAndConvertToInt(HWND hWnd)
	{
		WCHAR* text = new WCHAR[10];
		GetWindowText(hWnd, text, 10);
		int param = _wtoi(text);
		delete[] text;
		return param;
	}

	float degreeToRadians(int degree)
	{
		return 3.1415f * degree / 180.0f;
	}

	RECT getRect(int x, int y, int w, int h)
	{
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = w;
		rect.bottom = h;

		return rect;
	}

	class TextCaption
	{
	public:
		TextCaption(int x, int y, int w, int h) 
		{
			initRect(x, y, w, h);
		}

		TextCaption() {}

		void draw(HWND hwnd, LPCWSTR text, int x, int y, int w, int h)
		{
			initRect(x, y, w, h);
			draw(hwnd, text);
		}

		void draw(HWND hwnd, LPCWSTR text)
		{
			HDC hdc = GetWindowDC(hwnd);
			DrawText(hdc, text, -1, &rect, DT_SINGLELINE | DT_NOCLIP);
			DeleteDC(hdc);
		}
	private:
		void initRect(int x, int y, int w, int h)
		{
			rect.left = x;
			rect.top = y;
			rect.right = w;
			rect.bottom = h;
		}
		RECT rect;
	};
	const float g = 9.8f;
	const float alpha = 3.1415f * 45.0f / 180.0f;

	class ProcessRunnable : public Sys::Thread::Runnable
	{
	public:
		struct Vec2
		{
			Vec2() : x(0.0f), y(0.0f) {}
			Vec2(float _x, float _y) : x(_x), y(_y) {}

			float x;
			float y;
		};

		ProcessRunnable(int duration) :
			mProcessing(true),
			mDt(0.0f),
			mVel(100.0f),
			mAlpha(alpha),
			mDuration(duration),
			mResume(false),
			mStartTime(0),
			mPrevTime(0)
		{
		}

		virtual void run()
		{
			while (mProcessing)
			{
				if (isResumed())
				{
					if (mStartTime == 0)
					{
						mStartTime = Sys::Time::now();
						mPrevTime = mStartTime;
					}

					Sys::Time::Value currTime = Sys::Time::now();
					Sys::Time::Value frameTime = currTime - mPrevTime;

					if ((currTime - mStartTime) >= mDuration)
					{
						mResume = false;
						continue;
					}

					mCondVar.aquire();
					if (frameTime >= 16)
					{
						mDt += 0.1f;
						mRes = processNextStep(mDt);

						mPrevTime = currTime;

						mResume = true;
						mCondVar.signalAll();
					}
					
					mCondVar.release();
				}
				else
				{
					mStartTime = 0;
					mDt = 0.0f;

					mCondVar.signalAll();

					Sys::LockGuard<Sys::ConditionVariable> lock(mCondVar);
					while (!mResume)
						mCondVar.wait();
				}
			}
		}

		Vec2 get()
		{
			Sys::LockGuard<Sys::ConditionVariable> lock(mCondVar);
			mCondVar.wait();
			return mRes;
		}

		void start(float velocity, float angle)
		{
			Sys::LockGuard<Sys::ConditionVariable> lock(mCondVar);
			mVel = velocity;
			mAlpha = angle;
			mResume = true;
			mCondVar.signalAll();
		}

		void stop()
		{
			Sys::LockGuard<Sys::ConditionVariable> lock(mCondVar);
			mResume = false;
			mCondVar.signalAll();
		}

		bool isResumed()
		{
			Sys::LockGuard<Sys::ConditionVariable> lock(mCondVar);
			return mResume;
		}
	private:
		Vec2 processNextStep(float dt)
		{
			float vx = mVel * std::cos(mAlpha);
			float vy = mVel * std::sin(mAlpha);

			float x = vx * dt;
			float y = vy * dt - 0.5f * g * dt * dt;

			return Vec2(x, y);
		}

		bool mProcessing;
		float mDt;
		Vec2 mRes;
		float mVel;
		float mAlpha;
		int mDuration;
		bool mResume;
		Sys::ConditionVariable mCondVar;
		Sys::Time::Value mStartTime;
		Sys::Time::Value mPrevTime;
	};


	HWND App::createWindow(HINSTANCE hInstance, int nCmdShow, int width, int height)
	{
		WNDCLASSEX wc;
		HWND hwnd;

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = App::ThisWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"Class name";
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, L"Window Registration Failed!", L"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}

		hwnd = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"Class name",
			L"The title of my window",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			NULL, NULL, hInstance, (void*)this);

		if (hwnd == NULL)
		{
			MessageBox(NULL, L"Window Creation Failed!", L"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}

		return hwnd;
	}

	App::App(HINSTANCE hInstance, int nCmdShow, int width, int height) :
		m_hInstance(hInstance),
		m_nCmdShow(nCmdShow),
		m_width(width),
		m_height(height),
		mThread(0),
		mProcessRunnable(0)
	{
		mVelocityCaption = new TextCaption();
		mAngleCaption = new TextCaption();
	}

	App::~App()
	{
		mThread->join();

		SafeDeletePtr(mThread);
		SafeDeletePtr(mProcessRunnable);
		SafeDeletePtr(mVelocityCaption);
		SafeDeletePtr(mAngleCaption);
	}

	LRESULT CALLBACK App::ThisWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		App* app = NULL;

		if (msg == WM_CREATE)
		{
			CREATESTRUCT* cs = (CREATESTRUCT*) lParam;
			app = (App*)cs->lpCreateParams;

			SetLastError(0);
			if (SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)app) == 0)
			{
				if (GetLastError() != 0)
					return FALSE;
			}
		}
		else
		{
			app = (App*)GetWindowLongPtr(hwnd, GWL_USERDATA);
		}

		if (app)
			return app->WindowProc(msg, wParam, lParam);

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK App::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			break;
		case WM_PAINT:
			break;
		case WM_COMMAND:
			if ((HWND)lParam == mButton)
			{
				int velocity = getTextAndConvertToInt(mVelEditBox);
				int angleInDegree = getTextAndConvertToInt(mAngleEditBox);
				((ProcessRunnable*)mProcessRunnable)->start((float)velocity, degreeToRadians(angleInDegree));
			}
			break;
		case WM_START_ANIMATION:
			break;
		case WM_RENDER_WND:
			render();
			break;
		case WM_UPDATE_WND:
			InvalidateRect(mHwnd, NULL, TRUE);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(mHwnd, msg, wParam, lParam);
		}
	}

	void App::start()
	{
		init();

		MSG msg = { 0 };
		bool m_bIsGamestartning = true;
		while (msg.message != WM_QUIT && m_bIsGamestartning)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			update();
			render();
		}
	}

	void App::render()
	{
		ProcessRunnable::Vec2 res;
		ProcessRunnable* runnable = (ProcessRunnable*)mProcessRunnable;
		if (runnable->isResumed())
			res = runnable->get();
		else
		{
			return;
		}

		const int bottomBoundary = m_height - 80;
		RECT rect = getRect(0, 0, m_width, bottomBoundary);

		int cx = (int)(res.x / 2);
		int cy = (int)(res.y / 2);

		int middleH = m_height / 2;
		RECT rectPos = getRect(0 + cx, middleH - cy, 10 + cx, middleH + 10 - cy);
		if (middleH - cy > (bottomBoundary - (rectPos.bottom - rectPos.top)))
		{
			runnable->stop();
			return;
		}

		HDC hDC = GetWindowDC(mHwnd);
		
		FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW + 2));
		Rectangle(hDC, rectPos.left, rectPos.top, rectPos.right, rectPos.bottom);
		DeleteDC(hDC);
	}

	void App::update()
	{
	}

	void App::init()
	{
		mHwnd = createWindow(m_hInstance, m_nCmdShow, m_width, m_height);

		mButton = CreateWindow(
			L"BUTTON",
			L"Start", WS_VISIBLE | WS_CHILD, 10, m_height - 100,
			60, 40, mHwnd, NULL,
			(HINSTANCE)GetWindowLong(mHwnd, GWL_HINSTANCE),
			NULL);

		mVelEditBox = CreateWindow(L"edit", L"100",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 160, m_height - 80, 60, 20,
			mHwnd, NULL, (HINSTANCE)GetWindowLong(mHwnd, GWL_HINSTANCE), 0);

		mAngleEditBox = CreateWindow(L"edit", L"45",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 280, m_height - 80, 60, 20,
			mHwnd, NULL, (HINSTANCE)GetWindowLong(mHwnd, GWL_HINSTANCE), 0);

		ShowWindow(mHwnd, m_nCmdShow);
		UpdateWindow(mHwnd);

		mVelocityCaption->draw(mHwnd, L"Velocity:", 100, m_height - 45, 60, 20);
		mAngleCaption->draw(mHwnd, L"Angle:", 240, m_height - 45, 60, 20);

		mProcessRunnable = new ProcessRunnable(5000);
		mThread = new Sys::Thread(mProcessRunnable);
	}
}