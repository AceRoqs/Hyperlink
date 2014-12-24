#pragma once

// TODO: The code in this namespace will eventually be merged into WindowsCommon.
namespace HyperlinkControl
{

// Reference get_hyperlink_control_class when using CreateWindow to create
// a hyperlink control.
PCTSTR get_hyperlink_control_class();
HRESULT register_hyperlink_class(_In_ HINSTANCE instance);
void unregister_hyperlink_class(_In_ HINSTANCE instance);

}

