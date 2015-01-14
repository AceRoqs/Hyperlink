#pragma once

namespace WindowsCommon
{

// Reference get_hyperlink_control_class when using CreateWindow to create
// a hyperlink control.
PCWSTR get_hyperlink_control_class() NOEXCEPT;
HRESULT register_hyperlink_class(_In_ HINSTANCE instance) NOEXCEPT;
void unregister_hyperlink_class(_In_ HINSTANCE instance) NOEXCEPT;

}

