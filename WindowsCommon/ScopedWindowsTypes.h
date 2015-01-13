#pragma once

namespace WindowsCommon
{

template <typename RESOURCE, typename DELETER=std::function<void (RESOURCE)>>
class Scoped_resource
{
    DELETER m_deleter;
    RESOURCE m_resource;

    // Prevent copy.
    Scoped_resource& operator=(const Scoped_resource&) EQUALS_DELETE;
    Scoped_resource(const Scoped_resource&) EQUALS_DELETE;

public:
    explicit Scoped_resource() NOEXCEPT :
        m_resource(0)
    {
    }

    explicit Scoped_resource(RESOURCE resource, DELETER&& deleter) NOEXCEPT :
        m_deleter(std::move(deleter)),
        m_resource(resource)
    {
    }

    ~Scoped_resource()
    {
        invoke();
    }

    Scoped_resource(Scoped_resource&& other) NOEXCEPT :
        m_deleter(std::move(other.m_deleter)),
        m_resource(std::move(other.m_resource))
    {
        other.release();
    }

    Scoped_resource& operator=(Scoped_resource&& other) NOEXCEPT
    {
        // Handle A=A case.
        if(this != &other)
        {
            invoke();
            m_deleter = std::move(other.m_deleter);
            m_resource = std::move(other.m_resource);
            other.release();
        }

        return *this;
    }

    void invoke() NOEXCEPT
    {
        if(m_resource != 0)
        {
            m_deleter(m_resource);
            m_resource = 0;
        }
    }

    RESOURCE release() NOEXCEPT
    {
        RESOURCE resource = m_resource;
        m_resource = 0;

        return resource;
    }

    operator const RESOURCE&() const NOEXCEPT
    {
        return m_resource;
    }
};

typedef Scoped_resource<ATOM> Scoped_atom;
typedef Scoped_resource<HWND> Scoped_window;
typedef Scoped_resource<HDC> Scoped_device_context;
typedef Scoped_resource<HGLRC> Scoped_gl_context;
typedef Scoped_resource<HGLRC> Scoped_current_context;
typedef Scoped_resource<HANDLE> Scoped_handle;

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance);
Scoped_window make_scoped_window(_In_ HWND window);
Scoped_device_context make_scoped_device_context(_In_ HDC device_context, _In_ HWND window);
Scoped_gl_context make_scoped_gl_context(_In_ HGLRC gl_context);
Scoped_current_context make_scoped_current_context(_In_ HGLRC gl_context);
Scoped_handle make_scoped_handle(_In_ HANDLE handle);

}

