#include "PreCompile.h"
#include "Wrappers.h"       // Pick up forward declarations to ensure correctness.
#include "CheckHR.h"
#include "WindowMessages.h"
#include <PortableRuntime/Unicode.h>
#include <PortableRuntime/Tracing.h>

namespace WindowsCommon
{

LRESULT CALLBACK Window_procedure::static_window_proc(__in HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT
{
    PortableRuntime::dprintf("%s\n", string_from_window_message(message));

    // Sent by CreateWindow.
    if(message == WM_NCCREATE)
    {
        CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(l_param);

        // This function should never fail.
        const auto app = reinterpret_cast<Window_procedure*>(create_struct->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }

    LRESULT return_value;

    // GetWindowLongPtr should never fail.
    // The variable 'app' is not valid until WM_NCCREATE has been sent.
    const auto app = reinterpret_cast<Window_procedure*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if(app != nullptr)
    {
        return_value = app->window_proc(window, message, w_param, l_param);
    }
    else
    {
        return_value = DefWindowProc(window, message, w_param, l_param);
    }

    return return_value;
}

Window_class::Window_class(UINT style, _In_ WNDPROC window_proc, int class_extra, int window_extra, _In_ HINSTANCE instance, _In_opt_ HICON icon, _In_ HCURSOR cursor,
    _In_opt_ HBRUSH background, _In_opt_ PCSTR menu_name, _In_ PCSTR class_name, _In_opt_ HICON small_icon) :
    m_menu_name(menu_name ? PortableRuntime::utf16_from_utf8(menu_name) : L""),
    m_class_name(PortableRuntime::utf16_from_utf8(class_name))
{
    m_window_class.cbSize        = sizeof(m_window_class);
    m_window_class.style         = style;
    m_window_class.lpfnWndProc   = window_proc;
    m_window_class.cbClsExtra    = class_extra;
    m_window_class.cbWndExtra    = window_extra;
    m_window_class.hInstance     = instance;
    m_window_class.hIcon         = icon;
    m_window_class.hCursor       = cursor;
    m_window_class.hbrBackground = background;
    m_window_class.lpszMenuName  = m_menu_name.length() > 0 ? m_menu_name.c_str() : nullptr;
    m_window_class.lpszClassName = m_class_name.c_str();
    m_window_class.hIconSm       = small_icon;
}

Window_class::Window_class(const Window_class&& other) NOEXCEPT :
    m_window_class(other.m_window_class),
    m_menu_name(std::move(other.m_menu_name)),
    m_class_name(std::move(other.m_class_name))
{
    m_window_class.lpszMenuName  = m_menu_name.length() > 0 ? m_menu_name.c_str() : nullptr;
    m_window_class.lpszClassName = m_class_name.c_str();
}

Window_class::operator const WNDCLASSEXW&() const NOEXCEPT
{
    return m_window_class;
}

Window_class get_default_blank_window_class(_In_ HINSTANCE instance, _In_ WNDPROC window_proc, _In_ PCSTR window_class_name) NOEXCEPT
{
    Window_class window_class(CS_HREDRAW | CS_VREDRAW,
                              window_proc,
                              0,
                              0,
                              instance,
                              nullptr,
                              LoadCursor(nullptr, IDC_ARROW),
                              nullptr,
                              nullptr,
                              window_class_name,
                              nullptr);

    // Return value optimization expected.
    return window_class;
}

Scoped_atom register_window_class(const WNDCLASSEXW& window_class)
{
    const auto atom = RegisterClassExW(&window_class);

    if(0 == atom)
    {
        HRESULT hr = hresult_from_last_error();
        assert(HRESULT_FROM_WIN32(ERROR_CLASS_ALREADY_EXISTS) != hr);
        check_hr(hr);
    }

    return make_scoped_window_class(atom, window_class.hInstance);
}

Scoped_window create_window(
    _In_opt_ PCSTR class_name,
    _In_opt_ PCSTR window_name,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    _In_opt_ HWND parent,
    _In_opt_ HMENU menu,
    _In_opt_ HINSTANCE instance,
    _In_opt_ PVOID param)
{
    const auto class_name_utf16 = PortableRuntime::utf16_from_utf8(class_name);
    const auto window_name_utf16 = PortableRuntime::utf16_from_utf8(window_name);
    const auto window = CreateWindowW(class_name_utf16.c_str(), window_name_utf16.c_str(), style, x, y, width, height, parent, menu, instance, param);

    CHECK_WINDOWS_ERROR(nullptr != window);

    return make_scoped_window(window);
}

Scoped_window create_normal_window(_In_ PCSTR class_name, _In_ PCSTR window_name, int width, int height, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param)
{
    return create_window(
        class_name,             // class_name.
        window_name,            // window_name.
        WS_OVERLAPPEDWINDOW |
        WS_CLIPCHILDREN |
        WS_CLIPSIBLINGS,        // style.
        CW_USEDEFAULT,          // x.
        CW_USEDEFAULT,          // y.
        width,                  // width.
        height,                 // height.
        HWND_DESKTOP,           // parent.
        nullptr,                // menu.
        instance,               // instance.
        param);                 // param.
}

Scoped_device_context get_device_context(_In_ HWND window)
{
    const auto device_context = GetDC(window);
    CHECK_WITH_CUSTOM_HR(nullptr != device_context, E_FAIL);

    return make_scoped_device_context(device_context, window);
}

Scoped_gl_context create_gl_context(_In_ HDC device_context)
{
    const PIXELFORMATDESCRIPTOR descriptor =
    {
        sizeof(PIXELFORMATDESCRIPTOR),      // Size of this descriptor.
        1,                                  // Version number.
        PFD_DRAW_TO_WINDOW |                // Support window.
        PFD_SUPPORT_OPENGL |                // Support OpenGL.
        PFD_GENERIC_ACCELERATED |           // Support hardware acceleration.
        PFD_DOUBLEBUFFER,                   // Double buffered.
        PFD_TYPE_RGBA,                      // RGBA type.
        32,                                 // 32-bit color depth.
        0, 0, 0, 0, 0, 0,                   // Color bits ignored.
        0,                                  // No alpha buffer.
        0,                                  // Shift bit ignored.
        0,                                  // No accumulation buffer.
        0, 0, 0, 0,                         // Accum bits ignored.
        24,                                 // 24-bit z-buffer.
        8,                                  // 8-bit stencil buffer.
        0,                                  // No auxiliary buffer.
        PFD_MAIN_PLANE,                     // Main layer.
        0,                                  // Reserved.
        0, 0, 0                             // Layer masks ignored.
    };

    const int pixel_format = ChoosePixelFormat(device_context, &descriptor);
    check_windows_error(pixel_format != 0);

    check_windows_error(SetPixelFormat(device_context, pixel_format, &descriptor));

    const auto rendering_context = wglCreateContext(device_context);
    CHECK_WINDOWS_ERROR(nullptr != rendering_context);

    return make_scoped_gl_context(rendering_context);
}

Scoped_current_context create_current_context(_In_ HDC device_context, _In_ HGLRC gl_context)
{
    check_windows_error(wglMakeCurrent(device_context, gl_context));

    return make_scoped_current_context(gl_context);
}

Scoped_handle create_file(
    _In_ PCSTR file_name,
    DWORD desired_access,
    DWORD share_mode,
    _In_opt_ PSECURITY_ATTRIBUTES security_attributes,
    DWORD creation_disposition,
    DWORD flags,
    _In_opt_ HANDLE template_file)
{
    const auto handle = CreateFileW(PortableRuntime::utf16_from_utf8(file_name).c_str(),
                                    desired_access,
                                    share_mode,
                                    security_attributes,
                                    creation_disposition,
                                    flags,
                                    template_file);

    check_windows_error(INVALID_HANDLE_VALUE != handle);

    return make_scoped_handle(handle);
}

Scoped_handle create_event(
    _In_opt_ PSECURITY_ATTRIBUTES security_attributes,
    bool manual_reset,
    bool initial_state,
    _In_opt_ PCSTR name)
{
    const auto handle = CreateEventW(security_attributes,
                                     manual_reset,
                                     initial_state,
                                     name != nullptr ? PortableRuntime::utf16_from_utf8(name).c_str() : nullptr);

    check_windows_error(INVALID_HANDLE_VALUE != handle);
    assert(0 != handle);    // Per Win32 contract.
    _Analysis_assume_(0 != handle);

    return make_scoped_handle(handle);
}

}

