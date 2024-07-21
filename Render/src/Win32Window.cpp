#include "Win32Window.hpp"

#ifdef RENDER_USES_WINDOWS

SampleRender::Win32Window::Win32Window(uint32_t width, uint32_t height, std::string_view title) :
	m_Width(width), m_Height(height), m_WindowStyle(WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_MAXIMIZEBOX | WS_THICKFRAME),
	m_Minimized(false), m_Maximized(false), m_ShouldClose(false)
{
	std::string singleTitle = title.data();
	m_Title = std::wstring(singleTitle.begin(), singleTitle.end());

	HINSTANCE hInstance = GetModuleHandleW(nullptr);
	DWORD cmdShow = 10;
	RECT windowDimensions = { 0, 0, (LONG)m_Width, (LONG)m_Height };
	
	InstantiateWindowClass(&hInstance);
	RegisterClassExW(&m_WindowClass);

	AdjustDimensions(&windowDimensions);

	m_WindowHandle = CreateWindowExW
	(
		0,
		m_Title.c_str(),
		m_Title.c_str(),
		m_WindowStyle,
		100,
		100,
		windowDimensions.right - windowDimensions.left,
		windowDimensions.bottom - windowDimensions.top,
		nullptr,
		nullptr,
		m_WindowClass.hInstance,
		nullptr
	);

	SetWindowLongPtrW(m_WindowHandle, 0, (LONG_PTR)this);

	ShowWindow(m_WindowHandle, cmdShow);
	UpdateWindow(m_WindowHandle);
}

SampleRender::Win32Window::~Win32Window()
{
	DestroyWindow(m_WindowHandle);
	HINSTANCE hInstance = m_WindowClass.hInstance;
	UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);
	FreeModule(hInstance);
}

uint32_t SampleRender::Win32Window::GetWidth() const
{
	return m_Width;
}

uint32_t SampleRender::Win32Window::GetHeight() const
{
	return m_Height;
}

std::any SampleRender::Win32Window::GetNativePointer()
{
	return m_WindowHandle;
}

bool SampleRender::Win32Window::ShouldClose() const
{
	return m_ShouldClose;
}

bool SampleRender::Win32Window::IsMinimized() const
{
	return m_Minimized;
}

void SampleRender::Win32Window::Update()
{
	if (PeekMessageA(&m_MSG, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_MSG);
		DispatchMessageA(&m_MSG);
	}
}

void SampleRender::Win32Window::ConnectResizer(std::function<void(uint32_t, uint32_t)> resizer)
{
	m_Resizer = resizer;
}

void SampleRender::Win32Window::InstantiateWindowClass(HINSTANCE* instance)
{
	memset(&m_WindowClass, 0, sizeof(m_WindowClass));
	m_WindowClass.hInstance = *instance;
	m_WindowClass.cbSize = sizeof(m_WindowClass);
	m_WindowClass.cbClsExtra = 0;
	m_WindowClass.cbWndExtra = sizeof(void*);
	m_WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	//m_WindowClass.hIcon = (HICON)LoadImageA();
	m_WindowClass.hIcon = nullptr;
	m_WindowClass.hCursor = LoadCursorA(m_WindowClass.hInstance, IDC_ARROW);
	m_WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	m_WindowClass.lpszClassName = m_Title.c_str();
	m_WindowClass.lpszMenuName = nullptr;
	m_WindowClass.lpfnWndProc = Win32Callback::WindowResizer;
}

void SampleRender::Win32Window::AdjustDimensions(LPRECT windowDimensions)
{
	AdjustWindowRectEx(windowDimensions, m_WindowStyle, 0, 0);
}

void SampleRender::Win32Window::ApplyWindowResize(WPARAM wParam, LPRECT newDimensions)
{
	RECT windowDimensions = { 0, 0, (LONG)m_Width, (LONG)m_Height };
	AdjustWindowRectEx(&windowDimensions, m_WindowStyle, 0, 0);
	int32_t deltaWidth = (newDimensions->right - newDimensions->left) - (windowDimensions.right - windowDimensions.left);
	int32_t deltaHeight = (newDimensions->bottom - newDimensions->top) - (windowDimensions.bottom - windowDimensions.top);
	if(m_Resizer)
		m_Resizer((m_Width + deltaWidth), (m_Height + deltaHeight));
}

LRESULT SampleRender::Win32Callback::WindowResizer(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hWnd, 0));

	if (!window)
		return DefWindowProcW(hWnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_CLOSE:
	case WM_QUIT:
		window->m_ShouldClose = true;
		break;
	case WM_SIZE:
	{
		switch (wParam)
		{
		case SIZE_MINIMIZED:
		{
			window->m_Minimized = true;
			break;
		}
		case SIZE_MAXIMIZED:
		{
			window->ApplyWindowResize(wParam, (LPRECT)lParam);
			break;
		}
		case SIZE_RESTORED:
		{
			if (window->IsMinimized())
				window->m_Minimized = false;
			if (window->m_Maximized)
			{
				window->ApplyWindowResize(wParam, (LPRECT)lParam);
			}
			break;
		}
		}
		break;
	}
	case WM_SIZING:
	{
		window->ApplyWindowResize(wParam, (LPRECT)lParam);
		break;
	}
	default:
		break;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

#endif