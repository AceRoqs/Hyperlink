#include "PreCompile.h"
#include "WindowsGL.h"      // Pick up forward declarations to ensure correctness.
#include "CheckHR.h"
#include <PortableRuntime/Tracing.h>

namespace WindowsCommon
{

#ifndef NDEBUG // These functions are not currently called in Release.  Avoid C4505: unreferenced local function has been removed.
static std::vector<std::string> tokenize_string(_In_z_ const char* str, _In_z_ const char* delimiters)
{
    std::vector<std::string> tokens;

    const char* begin = str;
    const char* end = str + strlen(str);
    const char* delimiters_end = delimiters + strlen(delimiters);

    const char* iter;
    while((iter = std::find_first_of(begin, end, delimiters, delimiters_end)) != end)
    {
        tokens.push_back(std::string(begin, iter - begin));
        begin = iter + 1;
    }

    // Handle case where delimiter is not in string (usually the final token).
    if(begin < end)
    {
        tokens.push_back(std::string(begin));
    }

    return tokens;
}

static std::vector<std::string> get_opengl_extensions()
{
    // The VMware OpenGL Mesa driver includes an extra space on the end of the string.
    // The tokenizer works either way, but this appears to be working around buggy tokenizers that
    // miss the final OpenGL extension string.
    const char* extensions_string = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

    std::vector<std::string> extensions;
    if(extensions_string != nullptr)
    {
        extensions = tokenize_string(extensions_string, " ");
        std::sort(std::begin(extensions), std::end(extensions));
    }

    return extensions;
}
#endif

static void dprintf_gl_strings()
{
#ifndef NDEBUG
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    PortableRuntime::dprintf("OpenGL vendor: %s\n", vendor != nullptr ? vendor : "");

    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    PortableRuntime::dprintf("OpenGL renderer: %s\n", renderer != nullptr ? renderer : "");

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    PortableRuntime::dprintf("OpenGL version: %s\n", version != nullptr ? version : "");

    auto extensions = get_opengl_extensions();
    PortableRuntime::dprintf("OpenGL extensions:\n");
    std::for_each(extensions.cbegin(), extensions.cend(), [](const std::string& extension)
    {
        PortableRuntime::dprintf("%s\n", extension.c_str());
    });
#endif
}

static bool is_window_32bits_per_pixel(_In_ HWND window)
{
    Scoped_device_context device_context = get_device_context(window);

    if(::GetDeviceCaps(device_context, BITSPIXEL) < 32)
    {
        return false;
    }

    return true;
}

#if _MSC_VER <= 1800
WGL_state::WGL_state()
{
}

WGL_state::WGL_state(WGL_state&& other) :
    atom(std::move(other.atom)),
    window(std::move(other.window)),
    device_context(std::move(other.device_context)),
    gl_context(std::move(other.gl_context)),
    make_current_context(std::move(other.make_current_context))
{
}

WGL_state& WGL_state::operator=(WGL_state&& other) NOEXCEPT
{
    // Handle A=A case.
    if(this != &other)
    {
        atom = std::move(other.atom);
        window = std::move(other.window);
        device_context = std::move(other.device_context);
        gl_context = std::move(other.gl_context);
        make_current_context = std::move(other.make_current_context);
    }

    return *this;
}
#else
#error This compiler may autodefine the default move constructor.
#endif

// TODO: set window width/height if full screen
OpenGL_window::OpenGL_window(_In_ PCSTR window_title, _In_ HINSTANCE instance, bool windowed) : m_windowed(windowed)
{
    const int window_width = 800;
    const int window_height = 600;

    const Window_class window_class = get_default_blank_window_class(instance, Window_procedure::static_window_proc, window_title);

    m_state.atom = register_window_class(window_class);

    if(windowed)
    {
        m_state.window = create_normal_window(window_title, window_title, window_width, window_height, instance, this);
    }
    else
    {
        DEVMODE DevMode;
        ZeroMemory(&DevMode, sizeof(DEVMODE));
        DevMode.dmSize = sizeof(DEVMODE);
        DevMode.dmBitsPerPel = 32;
        DevMode.dmPelsWidth = 640;
        DevMode.dmPelsHeight = 480;
        DevMode.dmFields = DM_BITSPERPEL;

        ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
        DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);

        m_state.window = create_window(
            window_title,
            window_title,
            WS_POPUP | WS_CLIPSIBLINGS,
            0,
            0,
            DevMode.dmPelsWidth,
            DevMode.dmPelsHeight,
            nullptr,
            nullptr,
            instance,
            nullptr);

        ShowCursor(false);
    }

    // TODO: 2014: This message text is good - find some way to pass this via the exception.
    //MessageBox(window, TEXT("3D Engine demo requires 32-bit color."), TEXT("System requirements"), MB_OK);
    check_with_custom_hr(is_window_32bits_per_pixel(m_state.window), E_FAIL);

    m_state.device_context = get_device_context(m_state.window);
    m_state.gl_context = create_gl_context(m_state.device_context);
    m_state.make_current_context = create_current_context(m_state.device_context, m_state.gl_context);

    dprintf_gl_strings();
}

OpenGL_window::~OpenGL_window()
{
    // TODO: 2014: This is just a placeholder - the fullscreen OpenGL code isn't currently exercised.
    if(!m_windowed)
    {
        ::ChangeDisplaySettings(nullptr, 0);
    }
}

LRESULT OpenGL_window::window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT
{
    LRESULT return_value = 0;

    switch(message)
    {
        case WM_SIZE:
        {
            RECT client_rectangle;
            ::GetClientRect(window, &client_rectangle);

            ::glViewport(client_rectangle.left, client_rectangle.top, client_rectangle.right, client_rectangle.bottom);

            break;
        }

        case WM_ERASEBKGND:
        {
            // Do not erase background, as the whole window area will be refreshed anyway.
            break;
        }

        case WM_DESTROY:
        {
            m_state.make_current_context.invoke();
            m_state.gl_context.invoke();
            m_state.device_context.invoke();

            // No need to invoke destructor of window, as that would dispatch another WM_DESTROY.
            m_state.window.release();

            break;
        }

        default:
        {
            return_value = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return return_value;
}

}

