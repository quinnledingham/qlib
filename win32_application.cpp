global_variable bool32 GlobalRunning;

#ifndef WIN32_THREAD_H
#pragma message ("win32_application.cpp requires win32_thread.h")
#endif

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void 
Win32InitThreads(win32_thread_info *ThreadInfo, int InfoArrayCount, platform_work_queue *Queue)
{
    uint32 InitialCount = 0;
    uint32 ThreadCount = InfoArrayCount;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex) {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

internal void
Win32LoadXInput(void)    
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}
        
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
        
        // TODO(casey): Diagnostic
        
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, platform_button_state *OldState, DWORD ButtonBit, platform_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;
    
    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }
    
    return(Result);
}


internal platform_window_dimension
Win32GetWindowDimension(HWND Window)
{
    platform_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return(Result);
}

internal void
Win32ProcessKeyboardMessage(platform_button_state *NewState, bool32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        if (IsDown && !NewState->NewEndedDown) {
            NewState->NewEndedDown = true; 
        }
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
Win32ProcessKeyboardMessage(platform_button_state *NewState, platform_button_state *OldState, bool32 IsDown)
{
    if(NewState->EndedDown != IsDown) {
        NewState->EndedDown = IsDown;
        NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
    }
}

internal void
Win32ProcessPendingMessages(platform_controller_input *KeyboardController,
                            platform_controller_input *OldKeyboardController,
                            platform_keyboard_input *Keyboard,
                            platform_keyboard_input *OldKeyboard,
                            platform_input *Input)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                
                if (WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, &OldKeyboardController->MoveUp, IsDown);
                    }
                    else if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, &OldKeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, &OldKeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, &OldKeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == VK_F5)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->F5, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Up, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Left, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Down, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Right, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, &OldKeyboardController->Start, IsDown);
                    }
                    else if(VKCode >= '0' && VKCode <= '9')
                    {
                        int index = VKCode - '0';
                        Win32ProcessKeyboardMessage(&Keyboard->Numbers[index], &OldKeyboard->Numbers[index], IsDown);
                    }
                    else if (VKCode == VK_OEM_PERIOD)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Period, IsDown);
                    }
                    else if (VKCode == VK_BACK)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Backspace, &OldKeyboard->Backspace, IsDown);
                    }
                    else if (VKCode == VK_TAB)
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Tab, &OldKeyboard->Tab, IsDown);
                    }
                    else if (VKCode == VK_RETURN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Enter, IsDown);
                    }
                    
                    // alt-f4
                    bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                    if((VKCode == VK_F4) && AltKeyWasDown)
                    {
                        GlobalRunning = false;
                    }
                    
                    bool32 CrtlKeyWasDown = (Message.lParam & (1 << 17));
                    if ((VKCode == 'V') && CrtlKeyWasDown)
                    {
                        HGLOBAL hglb; 
                        OpenClipboard(0);
                        hglb = GetClipboardData(CF_TEXT);
                        if (hglb != 0) {
                            memcpy(&Keyboard->Clipboard, hglb, (int)GlobalSize(hglb));
                            Win32ProcessKeyboardMessage(&Keyboard->CtrlV, &OldKeyboard->CtrlV, IsDown);
                        }
                        CloseClipboard();;
                    }
                }
            } break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if (WasDown != IsDown) {
                    if (VKCode == MK_LBUTTON)
                        Win32ProcessKeyboardMessage(&Input->MouseButtons[0], IsDown);
                }
            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

internal void
Win32ResizeDIBSection(platform_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE(casey): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;
    
    // TODO(casey): Probably clear this to black
}

global_variable int64 GlobalPerfCountFrequency;

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

internal void
Win32DisplayBufferInWindow(platform_offscreen_buffer *Buffer, HDC DeviceContext, 
                           int WindowWidth, int WindowHeight)
{
    if((WindowWidth >= Buffer->Width*2) &&
       (WindowHeight >= Buffer->Height*2))
    {
        StretchDIBits(DeviceContext,
                      0, 0, 2*Buffer->Width, 2*Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        int OffsetX = 0;
        int OffsetY = 0;
        
        // NOTE(casey): For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!
        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

#ifdef QLIB_WINDOW_APPLICATION

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT MESSAGE, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(MESSAGE)
    {
        case WM_CLOSE:
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            if (gVertexArrayObject != 0) {
                HDC hdc = GetDC(Window);
                HGLRC hglrc = wglGetCurrentContext();
                
                glBindVertexArray(0);
                glDeleteVertexArrays(1, &gVertexArrayObject);
                gVertexArrayObject = 0;
                
                wglMakeCurrent(NULL, NULL);
                wglDeleteContext(hglrc);
                ReleaseDC(Window, hdc);
                
                PostQuitMessage(0);
            }
            else
            {
                
            }
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        case WM_PAINT:
        case WM_ERASEBKGND:
        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, MESSAGE, WParam, LParam);
        } break;
    }
    return(Result);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    SetCurrentDirectory("../game/data");
    
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = hInstance;
    
    WindowClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    WindowClass.hIconSm = (HICON)LoadImage(hInstance, "icon.ico", IMAGE_ICON, 100, 100, 
                                           LR_LOADFROMFILE | LR_LOADTRANSPARENT);
    
    //WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WindowClass.lpszMenuName = 0;
    WindowClass.lpszClassName = "Win32 Game Window";
    RegisterClassEx(&WindowClass);
    
    // Center window on screen
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int ClientWidth = 1000;
    int ClientHeight = 1000;
    RECT WindowRect;
    SetRect(&WindowRect,
            (ScreenWidth / 2) - (ClientWidth / 2),
            (ScreenHeight / 2) - (ClientHeight / 2),
            (ScreenWidth / 2) + (ClientWidth / 2),
            (ScreenHeight / 2) + (ClientHeight / 2));
    
    Win32LoadXInput();
    
    Win32ResizeDIBSection(&GlobalBackbuffer, ClientWidth, ClientHeight);
    
    DWORD Style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    // | WS_THICKFRAME to resize
    
    AdjustWindowRectEx(&WindowRect, Style, FALSE, 0);
    HWND hwnd = CreateWindowEx(0, WindowClass.lpszClassName,
                               "Coffee Cow", Style, WindowRect.left,
                               WindowRect.top, WindowRect.right -
                               WindowRect.left, WindowRect.bottom -
                               WindowRect.top, NULL, NULL,
                               hInstance, szCmdLine);
    HDC hdc = GetDC(hwnd);
    
    // Setting the pixel format
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);
    
    // Temporary OpenGL context to get a pointer to wglCreateContext
    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
    // Getting OpenGL 4.6 Core context profile
    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0, };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);
    
    // Loading OpenGL 4.6 Core functions with glad
    if (!gladLoadGL()) {
        PrintqDebug("Could not initialize GLAD\n");
    }
    else {
        PrintqDebug("OpenGL Version \n");//" + GLVersion.major + "." + GLVersion.minor + "\n");
        //std::cout <<  <<
        //GLVersion.major << "." << GLVersion.minor <<
        //"\n";
    }
    
    int vsynch = 0;
#if VSYNC
    // Enabling vsync
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = 
    (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
    
    if (swapControlSupported)
    {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = 
        (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        
        if (wglSwapIntervalEXT(1))
        {
            OutputDebugStringA("Enabled vsynch\n");
            vsynch = wglGetSwapIntervalEXT();
        }
        else
        {
            OutputDebugStringA("Could not enable vsynch\n");
        }
    }
    else
    { 
        // !swapControlSupported
        OutputDebugStringA("WGL_EXIT_swap_control not supported\n");
    }
#endif
    
    // Getting the global VAO
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    platform p = {};
    
#if QLIB_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    p.Memory.PermanentStorageSize = Gigabytes(1);
    p.Memory.TransientStorageSize = Megabytes(1);
    //p.Memory.PermanentStorageSize = Megabytes(256);
    //p.Memory.TransientStorageSize = Gigabytes(1);
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    GlobalDebugBuffer.Mutex = CreateMutex(NULL, FALSE, NULL);
    Manager.Mutex = CreateMutex(NULL, FALSE, NULL);
    
    if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        //platform_input Input[2] = {};
        //platform_input *NewInput = &Input[0];
        //platform_input *OldInput = &Input[1];
        platform_input *NewInput = &p.Inputs[0];
        platform_input *OldInput = &p.Inputs[1];
        
        NewInput->Keyboard.ControllerInput = &NewInput->Controllers[0];
        OldInput->Keyboard.ControllerInput = &OldInput->Controllers[0];
        
        // Gameplay Loop
        GlobalRunning = true;
        DWORD lastTick = GetTickCount();
        
        p.Initialized = false;
        
        // Multitheading
        win32_thread_info ThreadInfo[7];
        Win32InitThreads(ThreadInfo, ArrayCount(ThreadInfo), &p.Queue);
        
        //platform_controller_input *KeyboardController = &p.Input.Controllers[0];
        
        LARGE_INTEGER LastCounter =  Win32GetWallClock();
        
        while (GlobalRunning) {
            
            
            platform_keyboard_input *OldKeyboard = &OldInput->Keyboard;
            platform_keyboard_input *NewKeyboard = &NewInput->Keyboard;
            
            platform_controller_input *OldKeyboardController = OldKeyboard->ControllerInput;
            platform_controller_input *NewKeyboardController = NewKeyboard->ControllerInput;
            memset(NewKeyboardController->Buttons, 0, ArrayCount(NewKeyboardController->Buttons) * sizeof(platform_button_state));
            NewKeyboardController->IsConnected = true;
            for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
            {
                NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
            }
            
            memset(NewKeyboard->Buttons, 0, ArrayCount(NewKeyboard->Buttons) * sizeof(platform_button_state));
            for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboard->Buttons); ++ButtonIndex)
            {
                NewKeyboard->Buttons[ButtonIndex].EndedDown = OldKeyboard->Buttons[ButtonIndex].EndedDown;
            }
            
            memset(NewInput->MouseButtons, 0, sizeof(platform_button_state) * 5);
            Win32ProcessPendingMessages(NewKeyboardController, OldKeyboardController, NewKeyboard, OldKeyboard, NewInput);
            
#if QLIB_INTERNAL
            switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    // PrintqDebug - DebugBuffer
                    memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.MaxSize);
                    GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
                    GlobalDebugBuffer.Size = 0;
                }
                _finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
            }
#endif
            platform_offscreen_buffer Buffer = {};
            Buffer.Memory = GlobalBackbuffer.Memory;
            Buffer.Width = GlobalBackbuffer.Width;
            Buffer.Height = GlobalBackbuffer.Height;
            Buffer.Pitch = GlobalBackbuffer.Pitch;
            Buffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;
            
            p.Dimension = Win32GetWindowDimension(hwnd);
            
            // Mouse
            POINT MouseP;
            GetCursorPos(&MouseP);
            ScreenToClient(hwnd, &MouseP);
            
            LONG NewMouseX = MouseP.x - (p.Dimension.Width / 2); // Move origin to middle of screen;
            LONG NewMouseY = MouseP.y - (p.Dimension.Height / 2);
            if (NewMouseX != NewInput->MouseX || NewMouseY != NewInput->MouseY) {
                NewInput->MouseX = NewMouseX;
                NewInput->MouseY = NewMouseY;
                NewInput->MouseMoved = true;
            }
            else {
                NewInput->MouseMoved = false;
            }
            NewInput->MouseZ = 0; // TODO(casey): Support mousewheel?
            //Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
            //Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
            //Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
            //Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
            //Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
            
            // TODO(casey): Need to not poll disconnected controllers to avoid
            // xinput frame rate hit on older libraries...
            // TODO(casey): Should we poll this more frequently
            DWORD MaxControllerCount = XUSER_MAX_COUNT;
            if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
            {
                MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
            }
            
            for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
            {
                DWORD OurControllerIndex = ControllerIndex + 1;
                platform_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                platform_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                
                XINPUT_STATE ControllerState;
                if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                {
                    NewController->IsConnected = true;
                    NewController->IsAnalog = OldController->IsAnalog;
                    
                    // NOTE(casey): This controller is plugged in
                    // TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
                    XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                    
                    // TODO(casey): This is a square deadzone, check XInput to
                    // verify that the deadzone is "round" and show how to do
                    // round deadzone processing.
                    NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    
                    if((NewController->StickAverageX != 0.0f) ||
                       (NewController->StickAverageY != 0.0f))
                    {
                        NewController->IsAnalog = true;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                    {
                        NewController->StickAverageY = 1.0f;
                        NewController->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                    {
                        NewController->StickAverageY = -1.0f;
                        NewController->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                    {
                        NewController->StickAverageX = -1.0f;
                        NewController->IsAnalog = false;
                    }
                    
                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                    {
                        NewController->StickAverageX = 1.0f;
                        NewController->IsAnalog = false;
                    }
                    
                    real32 Threshold = 0.5f;
                    Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold) ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);
                    Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold) ? 1 : 0, &OldController->MoveRight, 1, &NewController->MoveRight);
                    Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold) ? 1 : 0, &OldController->MoveDown, 1, &NewController->MoveDown);
                    Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold) ? 1 : 0, &OldController->MoveUp, 1, &NewController->MoveUp);
                    
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Enter, XINPUT_GAMEPAD_A, &NewController->Enter);
                    Win32ProcessXInputDigitalButton(Pad->wButtons,&OldController->ActionRight, XINPUT_GAMEPAD_B, &NewController->ActionRight);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionLeft, XINPUT_GAMEPAD_X, &NewController->ActionLeft);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionUp, XINPUT_GAMEPAD_Y, &NewController->ActionUp);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, &NewController->LeftShoulder);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &NewController->RightShoulder);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Start, XINPUT_GAMEPAD_START, &NewController->Start);
                    Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Back, XINPUT_GAMEPAD_BACK, &NewController->Back);
                }
                else
                {
                    // NOTE(casey): The controller is not available
                    NewController->IsConnected = false;
                }
            }
            
            LARGE_INTEGER WorkCounter = Win32GetWallClock();
            NewInput->WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
            LastCounter =  Win32GetWallClock();
            
            //NewInput->CurrentInputInfo = OldInput->CurrentInputInfo;
            //NewInput->PreviousInputInfo = OldInput->PreviousInputInfo;
            NewInput->TriggerCount = OldInput->TriggerCount;
            p.Input = NewInput;
            if (p.Dimension.Width != 0 && p.Dimension.Height != 0)
                UpdateRender(&p);
            
            if (NewInput->NewCursor)
            {
                if (NewInput->Cursor == platform_cursor_mode::Arrow)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_ARROW);
                    SetCursor(curs); 
                }
                else if (NewInput->Cursor == platform_cursor_mode::Hand)
                {
                    HCURSOR curs = LoadCursor(NULL, IDC_HAND);
                    SetCursor(curs); 
                }
            }
            
#if QLIB_INTERNAL
            switch(WaitForSingleObject(GlobalDebugBuffer.Mutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    char CharBuffer[OUTPUTBUFFER_SIZE];
                    _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
                    OutputDebugStringA(CharBuffer);
                }
                _finally{if(!ReleaseMutex(GlobalDebugBuffer.Mutex)){}}break;case WAIT_ABANDONED:return false;
            }
#endif
            
            if (NewInput->Quit)
            {
                GlobalRunning = false;
            }
            
#if QLIB_OPENGL
            SwapBuffers(hdc);
            if (vsynch != 0) {
                glFinish();
            }
#else
            Win32DisplayBufferInWindow(&GlobalBackbuffer, hdc, p.Dimension.Width, p.Dimension.Height);
            if((GlobalBackbuffer.Width != p.Dimension.Width) || (GlobalBackbuffer.Height != p.Dimension.Height))
            {
                Win32ResizeDIBSection(&GlobalBackbuffer, p.Dimension.Width, p.Dimension.Height);
            }
#endif
            platform_input *Temp = NewInput;
            NewInput = OldInput;
            OldInput = Temp;
        } // End of game loop
    }
    else // if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        
    }
    
    return(0);
}
#endif // QLIB_WINDOW_APPLICATION

#ifdef QLIB_CONSOLE_APPLICATION
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv)
{
    platform p = {};
    
#if QLIB_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    p.Memory.PermanentStorageSize = Megabytes(100);
    p.Memory.TransientStorageSize = Megabytes(1);
    
    // TODO(casey): Handle various memory footprints (USING SYSTEM METRICS)
    uint64 TotalSize = p.Memory.PermanentStorageSize + p.Memory.TransientStorageSize;
    p.Memory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    p.Memory.TransientStorage = ((uint8 *)p.Memory.PermanentStorage + p.Memory.PermanentStorageSize);
    
    if (p.Memory.PermanentStorage && p.Memory.TransientStorage)
    {
        win32_thread_info ThreadInfo[7];
        Win32InitThreads(ThreadInfo, ArrayCount(ThreadInfo), &p.Queue);
        
        while(1)
        {
            Update(&p);
            
            char CharBuffer[OUTPUTBUFFER_SIZE];
            _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
            OutputDebugStringA(CharBuffer);
            
            GlobalDebugBuffer = {};
            memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
            GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
        }
    } // p.Memory.PermanentStorage && p.Memory.TransientStorage
    return (0);
}
#endif // QLIB_CONSOLE_APPLICATION


