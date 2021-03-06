This is a basic Hyperlink Win32 control.  It is a suitable control
for all Win32 systems, especially those which pre\-date the built\-in
common control.

In terms of dependencies, it relies on straight Win32 calls without
a dependency on MFC, ATL, or other frameworks.  It is standalone,
in that it doesn't subclass any Win32 controls.  It does take a soft
dependency on IDC\_HAND, which was introduced in Windows 2000, as
well as COLOR\_HOTLIGHT.

Given that the control is lightweight, it doesn't have support for
IME, RTL text, tooltips, parent notifications, or drag and drop.
It does have basic support for proper coloring, underlining,
hovering, and focus.

Direct access to the control class shouldn't be necessary, as the
control should be created either through CreateWindowEx or through
a .RC file.  Therefore, very little is exposed through the header
file.  `register_hyperlink_class()`, defined in Hyperlink.h, is
exposed to register the window class itself.  The HyperlinkTestApp
folder is a simple application which demonstrates this.

The code is a rather modern C++ interpretation of pure Win32 code,
including RAII for the GDI objects.  With that in mind, it does
require a compiler, such as Visual C++ 2015, which has C++11
support (particularly unique\_ptr).

Stylistically, the code is based on the JSF coding standards.  I
chose this as a reasonable bridge of styles between traditional
Windows coding and such modern C++ styles as used in Boost.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)

