#pragma once

namespace WindowsCommon
{

// Reference get_hyperlink_control_class when using CreateWindow to create
// a hyperlink control.
PCTSTR get_hyperlink_control_class();
HRESULT register_hyperlink_class(_In_ HINSTANCE instance);
void unregister_hyperlink_class(_In_ HINSTANCE instance);

}

