#include "PreCompile.h"
#include "Hyperlink.h"
#include "Resource.h"

//---------------------------------------------------------------------------
INT_PTR CALLBACK dialog_proc(_In_ HWND window,
                             UINT message,
                             WPARAM w_param,
                             LPARAM l_param)
{
    (l_param);  // Prevent unreferenced parameter.

    BOOL message_processed = FALSE;

    if(WM_COMMAND == message)
    {
        if((IDOK == LOWORD(w_param)) || (IDCANCEL == LOWORD(w_param)))
        {
            // Return 1 on success, as it makes the DialogBox* return code unambiguous.
            ::EndDialog(window, 1);
            message_processed = TRUE;
        }
    }

    return message_processed;
}

//---------------------------------------------------------------------------
int WINAPI _tWinMain(_In_ HINSTANCE instance,   // Handle to the program instance.
                     _In_opt_ HINSTANCE,        // hInstPrev - Unused in Win32.
                     _In_ PTSTR command_line,   // Command line.
                     _In_ int show_command)     // How the window is to be displayed.
{
    // Prevent unreferenced parameter.
    (command_line);
    (show_command);

    // Default the return code to 0, which is registered as ERRORLEVEL=0 in
    // a batch file.  This means running the app was successful.
    int return_code = 0;

    HyperlinkControl::register_hyperlink_class(instance);
    INT_PTR success = ::DialogBox(instance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, dialog_proc);
    HyperlinkControl::unregister_hyperlink_class(instance);

    // If success != 1, set the ERRORLEVEL to 1, indicating an error.
    return_code = (1 == success) ? 0 : 1;

    return return_code;
}
