#pragma once

// TODO: The code in this namespace will eventually be merged into WindowsCommon.
namespace HyperlinkControl
{

// Reference hyperlink_control_class when using CreateWindow to create
// a hyperlink control.
const PCTSTR hyperlink_control_class = TEXT("Hyperlink_control_class");

HRESULT register_hyperlink_class(_In_ HINSTANCE instance);
void unregister_hyperlink_class(_In_ HINSTANCE instance);

}

