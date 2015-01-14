#include "PreCompile.h"
#include "Hyperlink.h"  // Pick up forward declarations to ensure correctness.
#include "Hyperlink.rh"
#include <WindowsCommon/Wrappers.h>

namespace WindowsCommon
{

PCTSTR get_hyperlink_control_class() NOEXCEPT
{
    return HYPERLINK_CONTROL_CLASS;
}

class Hyperlink_control
{
public:
    Hyperlink_control(_In_ HWND window) NOEXCEPT;

protected:
    static LRESULT CALLBACK window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT;
    void on_set_font(_In_opt_ HFONT font, BOOL redraw) NOEXCEPT;
    void on_paint() NOEXCEPT;
    void on_focus() NOEXCEPT;
    void on_mouse_move(LONG x, LONG y) NOEXCEPT;
    void on_l_button_down(LONG x, LONG y) NOEXCEPT;
    void on_l_button_up(LONG x, LONG y) NOEXCEPT;
    void on_key_down(_In_ WPARAM key) NOEXCEPT;

    void navigate() NOEXCEPT;
    RECT get_hit_rect(_In_ HDC device_context) NOEXCEPT;
    bool is_in_hit_rect(LONG x, LONG y) NOEXCEPT;

private:
    HWND m_window;
    HFONT m_font;
    std::basic_string<TCHAR> m_link_name;

    // Not implemented to prevent accidental copying.
    Hyperlink_control(const Hyperlink_control&) EQUALS_DELETE;
    Hyperlink_control& operator=(const Hyperlink_control&) EQUALS_DELETE;

    // Required to avoid making window_proc public to all.
    friend HRESULT register_hyperlink_class(_In_ HINSTANCE instance);
};

HRESULT register_hyperlink_class(_In_ HINSTANCE instance) NOEXCEPT
{
    // This window class was derived by calling GetClassInfo on a 'static' control.
    WNDCLASSEX window_class;
    window_class.cbSize        = sizeof(window_class);
    window_class.style         = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS;
    window_class.lpfnWndProc   = Hyperlink_control::window_proc;
    window_class.cbClsExtra    = 0;
    window_class.cbWndExtra    = 0;
    window_class.hInstance     = instance;
    window_class.hIcon         = nullptr;
    window_class.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName  = nullptr;
    window_class.lpszClassName = get_hyperlink_control_class();
    window_class.hIconSm       = 0;

    HRESULT hr = S_OK;
    if(!RegisterClassEx(&window_class))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        assert(HRESULT_FROM_WIN32(ERROR_CLASS_ALREADY_EXISTS) != hr);
    }

    return hr;
}

void unregister_hyperlink_class(_In_ HINSTANCE instance) NOEXCEPT
{
    ::UnregisterClass(get_hyperlink_control_class(), instance);
}

static bool is_link_length_valid(const std::basic_string<TCHAR>& link_name) NOEXCEPT
{
    return link_name.length() < INT_MAX;
}

Hyperlink_control::Hyperlink_control(_In_ HWND window) NOEXCEPT :
    m_window(window),
    m_font(nullptr)
{
    assert(INVALID_HANDLE_VALUE != window);
}

LRESULT CALLBACK Hyperlink_control::window_proc(
    _In_ HWND window,           // Handle to the window.
    UINT message,               // Message that was sent.
    WPARAM w_param,             // First message parameter.
    LPARAM l_param) NOEXCEPT    // Second message parameter.
{
    LRESULT return_value = 0;

    // GetWindowLongPtr should never fail.
    // control is not valid until WM_NCCREATE has been sent.
    Hyperlink_control* control = reinterpret_cast<Hyperlink_control*>(::GetWindowLongPtr(window, GWLP_USERDATA));

    switch(message)
    {
        // Sent by CreateWindow.
        case WM_NCCREATE:
        {
            try
            {
                std::unique_ptr<Hyperlink_control> new_control = std::make_unique<Hyperlink_control>(window);

                CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(l_param);
                std::basic_string<TCHAR> link_name(create_struct->lpszName);

                if(is_link_length_valid(link_name))
                {
                    std::swap(link_name, new_control->m_link_name);
                    ::SetWindowLongPtr(window,
                                       GWLP_USERDATA,
                                       reinterpret_cast<LONG_PTR>(new_control.release()));

                    // Indicate that the window creation succeeded and that CreateWindow
                    // should NOT return a nullptr handle.
                    return_value = 1;
                }
            }
            catch(const std::bad_alloc& ex)
            {
                UNREFERENCED_PARAMETER(ex);

                // No modification to return_value implies error.
            }

            break;
        }

        case WM_NCDESTROY:
        {
            ::SetWindowLongPtr(window, GWLP_USERDATA, 0);
            delete control;
            control = nullptr;

            break;
        }

        case WM_SETTEXT:
        {
            try
            {
                std::basic_string<TCHAR> link_name(reinterpret_cast<PCTSTR>(l_param));

                if(is_link_length_valid(link_name))
                {
                    // Ensure that setting the title text of the control and saving the
                    // text in a member variable is atomic.
                    return_value = ::DefWindowProc(window, message, w_param, l_param);
                    if(return_value)
                    {
                        std::swap(link_name, control->m_link_name);
                    }
                }
            }
            catch(const std::bad_alloc& ex)
            {
                UNREFERENCED_PARAMETER(ex);

                // No modification to return_value implies error.
            }

            break;
        }

        case WM_SETFONT:
        {
            control->on_set_font(reinterpret_cast<HFONT>(w_param), LOWORD(l_param));
            break;
        }

        case WM_PAINT:
        {
            control->on_paint();
            break;
        }

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        {
            control->on_focus();
            break;
        }

        case WM_MOUSEMOVE:
        {
            control->on_mouse_move(GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
            break;
        }

        case WM_LBUTTONDOWN:
        {
            control->on_l_button_down(GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
            break;
        }

        case WM_LBUTTONUP:
        {
            control->on_l_button_up(GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
            break;
        }

        case WM_KEYDOWN:
        {
            control->on_key_down(w_param);
            break;
        }

        case WM_GETDLGCODE:
        {
            return_value = ::DefWindowProc(window, message, w_param, l_param);

            // By default, the dialog box will send VK_RETURN to the default control.
            // This can be handled by the owner window (http://support.microsoft.com/kb/102589),
            // or by handling WM_GETDLGCODE.
            MSG* msg = reinterpret_cast<MSG*>(l_param);
            if((nullptr != msg) && (VK_RETURN == w_param))
            {
                if(WM_KEYDOWN == msg->message)
                {
                    return_value |= DLGC_WANTALLKEYS;
                }
                else if(WM_CHAR == msg->message)
                {
                    // Prevent dialog box from beeping when receiving VK_RETURN.
                    // This can happen if navigate() fails and doesn't change the window focus.
                    return_value |= DLGC_WANTMESSAGE;
                }
            }

            break;
        }

        default:
        {
            return_value = ::DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return return_value;
}

void Hyperlink_control::on_set_font(_In_opt_ HFONT font, BOOL redraw) NOEXCEPT
{
    m_font = font;
    if(redraw)
    {
        ::InvalidateRect(m_window, nullptr, TRUE);
    }
}

void Hyperlink_control::on_paint() NOEXCEPT
{
    // NOTE: Send WM_CTLCOLORSTATIC to parent here if necessary.
    PAINTSTRUCT paint_struct;
    const HWND window = m_window;   // Lambda cannot capture member variables.
    std::unique_ptr<HDC__, std::function<void (HDC)>> context(
        ::BeginPaint(window, &paint_struct),
        [window, &paint_struct](HDC)
        {
            ::EndPaint(window, &paint_struct);
        });

    // Get the hyperlink color, but if it does not exist, then
    // default to the blue-ish color as default on Win7.
    DWORD color = ::GetSysColor(COLOR_HOTLIGHT);
    if(nullptr == ::GetSysColorBrush(COLOR_HOTLIGHT))
    {
        color = RGB(0, 102, 204);
    }

    ::SetTextColor(context.get(), color);
    ::SetBkMode(context.get(), TRANSPARENT);

    // Hyperlink_control uses the parent font sent via WM_SETFONT.
    HFONT current_font = m_font;
    if(nullptr == current_font)
    {
        current_font = static_cast<HFONT>(::GetCurrentObject(context.get(), OBJ_FONT));
    }

    LOGFONT log_font;
    ::GetObject(current_font, sizeof(log_font), &log_font);

    log_font.lfUnderline = TRUE;
    std::unique_ptr<HFONT__, std::function<void (HFONT)>> underline_font(
        ::CreateFontIndirect(&log_font),
        [](HFONT underline_font)
        {
            ::DeleteObject(underline_font);
        });
    std::unique_ptr<HFONT__, std::function<void (HFONT)>> old_font(
        SelectFont(context.get(), underline_font.get()),
        [&context](HFONT old_font)
        {
            ::SelectObject(context.get(), old_font);
        });

    RECT client_rect;
    ::GetClientRect(m_window, &client_rect);

    // ExtTextOut documentation specifies the character count as cbCount,
    // which implies count of bytes.
    // http://msdn.microsoft.com/en-us/library/dd162713%28v=vs.85%29.aspx
    // However, this is clarified on MSDN and in the SAL annotation of
    // ExtTextOut as being a count of characters:
    // http://msdn.microsoft.com/en-us/library/dd145112%28v=vs.85%29.aspx
    // ExtTextOut is used instead of TextOut so that the text is properly clipped.
    ::ExtTextOut(context.get(),                 // hdc
                 client_rect.left,              // X
                 client_rect.top,               // Y
                 ETO_CLIPPED,                   // options
                 &client_rect,                  // clip rectangle
                 m_link_name.c_str(),           // string
                 static_cast<UINT>(m_link_name.length()),   // character count
                 nullptr);                      // distance between origins of cells
}

void Hyperlink_control::on_focus() NOEXCEPT
{
    const auto device_context = get_device_context(m_window);
    const RECT hit_rect = get_hit_rect(device_context);

    // DrawFocusRect is an XOR operation, so the same call is used
    // for set focus and remove focus.
    ::DrawFocusRect(device_context, &hit_rect);
}

void Hyperlink_control::on_mouse_move(LONG x, LONG y) NOEXCEPT
{
    if(is_in_hit_rect(x, y))
    {
        // hInstance must be nullptr in order to use a predefined cursor.
        // NOTE: IDC_HAND is only available starting on Windows 2000.
        ::SetCursor(::LoadCursor(nullptr, IDC_HAND));
    }
}

void Hyperlink_control::on_l_button_down(LONG x, LONG y) NOEXCEPT
{
    if(is_in_hit_rect(x, y))
    {
        ::SetFocus(m_window);
        ::SetCapture(m_window);
    }
}

void Hyperlink_control::on_l_button_up(LONG x, LONG y) NOEXCEPT
{
    // Only navigate when the mouse is captured to prevent navigation
    // from happening when the button is pressed outside the hit box,
    // but released inside the hit box.
    if(::GetCapture() == m_window)
    {
        ::ReleaseCapture();

        if(is_in_hit_rect(x, y))
        {
            navigate();
        }
    }
}

void Hyperlink_control::on_key_down(_In_ WPARAM key) NOEXCEPT
{
    if((VK_SPACE == key) || (VK_RETURN == key))
    {
        navigate();
    }
}

void Hyperlink_control::navigate() NOEXCEPT
{
    // The August 1997 WDJ describes an implementation of GotoURL that
    // attempts to call WinExec() if ShellExecute() fails.  Use the simple
    // version until a modern browser is discovered where it doesn't work.
    // http://www.drdobbs.com/184416463
    ::ShellExecute(m_window,            // hwnd
                   TEXT("open"),        // lpOperation
                   m_link_name.c_str(), // lpFile
                   nullptr,             // lpParameters
                   nullptr,             // lpDirectory
                   SW_SHOWNORMAL);      // nShowCmd
}

RECT Hyperlink_control::get_hit_rect(_In_ HDC device_context) NOEXCEPT
{
    RECT hit_rect;

    std::unique_ptr<HFONT__, std::function<void (HFONT)>> font(
        SelectFont(device_context, m_font),
        [device_context](HFONT font)
        {
            ::SelectObject(device_context, font);
        });

    SIZE size;
    ::GetTextExtentPoint32(device_context,
                           m_link_name.c_str(),
                           static_cast<int>(m_link_name.length()),
                           &size);

    // Clip the text extents to the client rectangle.
    ::GetClientRect(m_window, &hit_rect);
    hit_rect.right = std::min(hit_rect.right, hit_rect.left + size.cx);
    hit_rect.bottom = std::min(hit_rect.bottom, hit_rect.top + size.cy);

    return hit_rect;
}

bool Hyperlink_control::is_in_hit_rect(LONG x, LONG y) NOEXCEPT
{
    bool is_in_hit_rect = false;

    const auto device_context = get_device_context(m_window);
    const RECT hit_rect = get_hit_rect(device_context);

    const POINT mouse_point = {x, y};
    if(::PtInRect(&hit_rect, mouse_point))
    {
        is_in_hit_rect = true;
    }

    return is_in_hit_rect;
}

}

