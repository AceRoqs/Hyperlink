/*
Copyright (C) 2011 by Toby Jones.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <tchar.h>
#include "hyperlink.h"
#include "resource.h"

//---------------------------------------------------------------------------
INT_PTR CALLBACK dialog_proc(__in HWND window,
                             UINT message,
                             WPARAM w_param,
                             LPARAM l_param)
{
    // Prevent unreferenced parameter.
    (l_param);

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

