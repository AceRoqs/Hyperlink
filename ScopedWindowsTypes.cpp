#include "PreCompile.h"
#include "ScopedWindowsTypes.h"     // Pick up forward declarations to ensure correctness.
#include "CheckHR.h"

namespace WindowsCommon
{

static void unregister_atom(_In_ ATOM atom, _In_ HINSTANCE instance) NOEXCEPT
{
    BOOL result = UnregisterClass(MAKEINTATOM(atom), instance);
    if(!result)
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

static std::function<void (ATOM)> unregister_class_functor(_In_ HINSTANCE instance) NOEXCEPT
{
    return [=](ATOM atom)
    {
        unregister_atom(atom, instance);
    };
}

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance)
{
    return std::move(Scoped_atom(atom, unregister_class_functor(instance)));
}

static void destroy_window(_In_ HWND window) NOEXCEPT
{
    if(!DestroyWindow(window))
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_window make_scoped_window(_In_ HWND window)
{
    return std::move(Scoped_window(window, std::function<void (HWND)>(destroy_window)));
}

static void release_device_context(_In_ HDC device_context, _In_ HWND window) NOEXCEPT
{
    if(!ReleaseDC(window, device_context))
    {
        assert(false);
    }
}

static std::function<void (HDC)> release_device_context_functor(_In_ HWND window) NOEXCEPT
{
    return [=](HDC device_context)
    {
        release_device_context(device_context, window);
    };
}

Scoped_device_context make_scoped_device_context(_In_ HDC device_context, _In_ HWND window)
{
    return std::move(Scoped_device_context(device_context, release_device_context_functor(window)));
}

static void delete_gl_context(_In_ HGLRC gl_context) NOEXCEPT
{
    if(!wglDeleteContext(gl_context))
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_gl_context make_scoped_gl_context(_In_ HGLRC gl_context)
{
    return std::move(Scoped_gl_context(gl_context, std::function<void (HGLRC)>(delete_gl_context)));
}

static void clear_gl_context(_In_opt_ HGLRC gl_context) NOEXCEPT
{
    UNREFERENCED_PARAMETER(gl_context);

    if(!wglMakeCurrent(nullptr, nullptr))
    {
        HRESULT hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_current_context make_scoped_current_context(_In_ HGLRC gl_context)
{
    // TODO: I can't think of a better way than to pass a gl_context, even though it is unused.
    // A non-null variable is required for the deleter to be part of move construction.
    return std::move(Scoped_current_context(gl_context, std::function<void (HGLRC)>(clear_gl_context)));
}

static void close_handle(_In_ HANDLE handle) NOEXCEPT
{
    if(!CloseHandle(handle))
    {
        auto hr = hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

Scoped_handle make_scoped_handle(_In_ HANDLE handle)
{
    return std::move(Scoped_handle(handle, std::function<void (HANDLE)>(close_handle)));
}

}

