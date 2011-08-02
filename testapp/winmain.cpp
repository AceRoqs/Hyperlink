/*
Copyright (C) 2011 by Toby Jones.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <windows.h>
#include <tchar.h>
#include "hyperlink.h"
#include "resource.h"

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
// Declspec SAL is used instead of attribute SAL, as the WinMain declaration
// in the system headers still uses declspec SAL.
int WINAPI _tWinMain(__in HINSTANCE instance,   // Handle to the program instance.
                     HINSTANCE,                 // hInstPrev - Unused in Win32.
                     __in PTSTR command_line,   // Command line.
                     int show_command)          // How the window is to be displayed.
{
    // Prevent unreferenced parameter.
    (command_line);
    (show_command);

    // Default the return code to 0, which is registered as ERRORLEVEL=0 in
    // a batch file.  This means running the app was successful.
    int return_code = 0;

    register_hyperlink_class(instance);
    INT_PTR success = ::DialogBox(instance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, dialog_proc);
    unregister_hyperlink_class(instance);

    // If success != 1, set the ERRORLEVEL to 1, indicating an error.
    return_code = (1 == success) ? 0 : 1;

    return return_code;
}

