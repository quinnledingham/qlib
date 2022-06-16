#ifndef APPLICATION_H
#define APPLICATION_H

/*
  NOTE(casey): Services that the platform layer provides to the game
*/
#if SNAKE_INTERNAL
/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
#endif
/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// Start Possible Win32
// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct platform_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};
global_variable platform_offscreen_buffer GlobalBackbuffer;

struct platform_window_dimension
{
    int Width;
    int Height;
};
// End Win32

struct platform_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

struct platform_button_state
{
    bool32 NewEndedDown;
    bool32 EndedDown;
};

struct platform_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;
    
    union
    {
        platform_button_state Buttons[12];
        struct
        {
            platform_button_state MoveUp;
            platform_button_state MoveDown;
            platform_button_state MoveLeft;
            platform_button_state MoveRight;
            
            platform_button_state ActionUp;
            platform_button_state ActionDown;
            platform_button_state ActionLeft;
            platform_button_state ActionRight;
            
            platform_button_state LeftShoulder;
            platform_button_state RightShoulder;
            
            platform_button_state Back;
            platform_button_state Start;
            
            // NOTE(casey): All buttons must be added above this line
            
            platform_button_state Terminator;
        };
    };
    
    
};

struct platform_keyboard_input
{
    union
    {
        platform_button_state Buttons[14];
        struct
        {
            platform_button_state Escape;
            platform_button_state Period;
            platform_button_state Backspace;
            platform_button_state F5;
            union
            {
                platform_button_state Numbers[10];
                struct
                {
                    platform_button_state Zero;
                    platform_button_state One;
                    platform_button_state Two;
                    platform_button_state Three;
                    platform_button_state Four;
                    platform_button_state Five;
                    platform_button_state Six;
                    platform_button_state Seven;
                    platform_button_state Eight;
                    platform_button_state Nine;
                };
            };
        };
    };
};

enum
CursorMode
{
    Arrow,
    Hand,
};

struct
platform_input
{
    platform_button_state MouseButtons[5];
    int32 MouseX, MouseY, MouseZ;
    CursorMode Cursor;
    bool32 NewCursor;
    
    float dt;
    real32 WorkSecondsElapsed;
    real32 Seconds;
    
    platform_controller_input Controllers[5];
    platform_keyboard_input Keyboard;
    
    int Quit;
};

inline platform_controller_input *GetController(platform_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    platform_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

inline platform_keyboard_input *GetKeyboard(platform_input *Input, int unsigned KeyboardIndex)
{
    platform_keyboard_input *Result = &Input->Keyboard;
    return Result;
}

struct
platform_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup
    
    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero at startup
};

struct
platform
{
    bool32 Initialized;
    
    platform_input Input;
    platform_memory Memory;
    platform_window_dimension Dimension;
};

#define OUTPUTBUFFER_SIZE 1000

// PrintqDebug
struct platform_debug_buffer
{
    int Size = OUTPUTBUFFER_SIZE;
    char Data[OUTPUTBUFFER_SIZE];
    char* Next;
};
global_variable platform_debug_buffer GlobalDebugBuffer = {};

struct
Camera
{
    v3 Position;
    v3 Target;
    v3 Up;
    
    float inAspectRatio;
    Shader* shader;
    float FOV;
    float F;
    platform_window_dimension Dimension;
};

void UpdateRender(platform* p);

#define BITMAP_BYTES_PER_PIXEL 4

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    int32 Pitch;
    void *Memory;
    
    void *Free;
};

internal void
RenderBitmap(loaded_bitmap *Bitmap, real32 RealX, real32 RealY);

#endif //APPLICATION_H
