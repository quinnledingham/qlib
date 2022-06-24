#ifndef MENU_H
#define MENU_H

enum struct menu_direction
{
    Right,
    Up,
    Left,
    Down,
};

struct menu_button
{
    font_string FontString;
    real32 DefaultTextPixelHeight;
    
    uint32 CurrentColor;
    uint32 DefaultColor;
    uint32 HoverColor;
    
    //uint32 CurrentTextColor;
    uint32 DefaultTextColor;
    uint32 HoverTextColor;
};

struct menu_logo
{
    Texture *Tex;
};

struct menu_text
{
    font_string FontString;
    uint32 DefaultTextColor;
};

struct menu_textbox
{
    font_string FontString;
    v2 TextCoords;
    
    bool Active;
    int MaxTextLength = 100;
    real32 XAdvances[100];
    
    int CursorPosition;
    real32 DisplayLeft;
    real32 DisplayRight;
    bool PaddingRight;
    
    uint32 CurrentColor;
    uint32 CurrentTextColor;
};

//enum menu_component_id;
enum struct menu_component_type
{
    Button,
    Text,
    TextBox,
    Logo,
};
struct menu_component
{
    int ID;
    
    v2 Coords;
    v2 GridCoords;
    v2 Dim;
    v2 DefaultDim;
    real32 DefaultTextPixelHeight;
    v2 PaddingDim;
    
    bool Active;
    
    menu_component *AlignWith;
    
    menu_component *NextSameType;
    menu_component_type Type;
    void *Data;
};

#define GetComponentData(d, t) ((t*)d->Data)

struct menu_events
{
    int ButtonClicked;
    int TextBoxClicked;
    bool ButtonHoverFlag;
};

struct menu_grid_row
{
    v2 Dim;
    real32 MenuGridColumnWidth[10];
};
struct menu
{
    bool Initialized;
    bool Reset;
    menu_grid_row Rows[10];
    
    v2 Coords;
    v2 Dim;
    
    v2 DefaultDim;
    v2 ScreenDim;
    
    int Padding;
    int DefaultPadding;
    
    int NumOfComponents;
    int MaxNumOfComponents;
    menu_component Components[20];
    menu_component *Buttons;
    menu_component *TextBoxes;
    menu_component *Texts;
    menu_component *Logos;
    
    menu_events Events;
    
    int ActiveIndex;
    int NumOfActiveComponents;
    menu_component *ActiveComponents[20];
    
    uint32 BackgroundColor;
};
inline void IncrActive(menu *Menu)
{
    Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
    Menu->ActiveIndex++;
    if (Menu->ActiveIndex == Menu->NumOfActiveComponents)
        Menu->ActiveIndex = 0;
}
inline void DecrActive(menu *Menu)
{
    Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
    Menu->ActiveIndex--;
    if (Menu->ActiveIndex < 0)
        Menu->ActiveIndex = Menu->NumOfActiveComponents - 1;
}
inline void FindActive(menu *Menu)
{
    for (int i = 0; i < Menu->NumOfActiveComponents; i++) {
        menu_component *C = Menu->ActiveComponents[i];
        if (C->Active)
            Menu->ActiveIndex = i;
    }
}

inline bool CoordsInRect( v2 Coords, v2 RectCoords, v2 RectDim)
{
    if (RectCoords.x < Coords.x && Coords.x < (RectCoords.x + RectDim.x) &&
        RectCoords.y < Coords.y && Coords.y < (RectCoords.y + RectDim.y))
        return true;
    
    return false;
}

internal menu_component*
MenuAddToComponentList(menu_component *Start, menu_component *NewComponent)
{
    if(Start == 0)
        Start = NewComponent;
    else {
        menu_component* Cursor = Start;
        while(Cursor->NextSameType != 0)
            Cursor = Cursor->NextSameType;
        Cursor->NextSameType = (menu_component*)NewComponent;
    }
    
    return Start;
}

internal menu_component*
MenuGetNextComponent(menu *Menu)
{
    Assert(Menu->NumOfComponents + 1 < Menu->MaxNumOfComponents);
    return &Menu->Components[Menu->NumOfComponents++];
}

// menu_button
internal int
MenuButtonClicked(menu *Menu, v2 MouseCoords)
{
    menu_component* Cursor = Menu->Buttons;
    while(Cursor != 0) {
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            return Cursor->ID;
        }
        Cursor = Cursor->NextSameType;
    }
    return -1;
}

internal bool
MenuButtonHovered(menu *Menu, v2 MouseCoords)
{
    bool ButtonHovered = false;
    
    menu_component* Cursor = Menu->Buttons;
    while(Cursor != 0) {
        menu_button *Button = (menu_button*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            //Button->CurrentColor = Button->HoverColor;
            //Button->FontString.Color = Button->HoverTextColor;
            Cursor->Active = true;
            ButtonHovered = true;
        }
        else {
            Cursor->Active = false;
            //Button->CurrentColor = Button->DefaultColor;
            //Button->FontString.Color = Button->DefaultTextColor;
        }
        Cursor = Cursor->NextSameType;
    }
    
    return ButtonHovered;
}

internal void
MenuResizeButton(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_button *Button = (menu_button*)MenuComponent->Data;
    FontStringResize(&Button->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddButton(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_button *Button)
{
    Button->FontString.Color = Button->DefaultTextColor;
    FontStringInit(&Button->FontString);
    
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = Button->FontString.PixelHeight;
    MenuComponent->Type = menu_component_type::Button;
    MenuComponent->ID = ID;
    
    Button->CurrentColor = Button->DefaultColor;
    
    MenuComponent->Data = qalloc((void*)Button, sizeof(menu_button));
    MenuResizeButton(Menu, MenuComponent, 0);
    
    
    Menu->Buttons = MenuAddToComponentList(Menu->Buttons, MenuComponent);
    return MenuComponent;
}

// menu_text
internal void
MenuResizeText(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_text *Text = (menu_text*)MenuComponent->Data;
    MenuComponent->Dim = FontStringResize(&Text->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_text *Text, menu_component *Align)
{
    Text->FontString.Color = Text->DefaultTextColor;
    FontStringInit(&Text->FontString);
    
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = FontStringGetDim(&Text->FontString);
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = Text->FontString.PixelHeight;
    MenuComponent->AlignWith = Align;
    MenuComponent->Type = menu_component_type::Text;
    MenuComponent->ID = ID;
    
    MenuComponent->Data = qalloc((void*)Text, sizeof(menu_text));
    MenuResizeText(Menu, MenuComponent, 0);
    
    Menu->Texts = MenuAddToComponentList(Menu->Texts, MenuComponent);
    return MenuComponent;
}
inline menu_component* MenuAddText(menu *Menu, int ID, v2 GridCoords, menu_text *Text)
{
    return MenuAddText(Menu, ID, GridCoords, Text, 0);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_text *Text, menu_component *Align)
{
    return MenuAddText(Menu, -1, GridCoords, Text, Align);
}
inline menu_component* MenuAddText(menu *Menu, v2 GridCoords, menu_text *Text)
{
    return MenuAddText(Menu, -1, GridCoords, Text, 0);
}

// menu_textbox
inline void IncrementCursorPosition(menu_textbox *TextBox)
{
    if (TextBox->CursorPosition < TextBox->FontString.Length) {
        TextBox->CursorPosition++;
    }
}
inline void DecrementCursorPosition(menu_textbox *TextBox)
{
    if (TextBox->CursorPosition != 0) {
        TextBox->CursorPosition--;
    }
}

internal int
MenuTextBoxClicked(menu *Menu, v2 MouseCoords)
{
    int ID = -1;
    
    menu_component* Cursor = Menu->TextBoxes;
    while(Cursor != 0) {
        menu_textbox *TextBox = (menu_textbox*)Cursor->Data;
        if (CoordsInRect(MouseCoords, Cursor->Coords, Cursor->Dim)) {
            Cursor->Active = true;
            ID = Cursor->ID;
        }
        else
            Cursor->Active = false;
        Cursor = Cursor->NextSameType;
    }
    
    return ID;
}

internal menu_component*
MenuTextBoxGetActive(menu_component* Cursor)
{
    while(Cursor != 0) {
        menu_textbox *TextBox = (menu_textbox*)Cursor->Data;
        if (TextBox->Active)
            return Cursor;
        Cursor = Cursor->NextSameType;
    }
    return 0;
}

internal void
MenuTextBoxArrowKeysMoveCursor(menu_component *MenuComponent,  menu_direction Dir)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (Dir == menu_direction::Right) 
        IncrementCursorPosition(TextBox);
    else if (Dir == menu_direction::Left) 
        DecrementCursorPosition(TextBox);
}

internal void
MenuTextBoxMouseMoveCursor(menu_component *MenuComponent, v2 MouseCoords)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    
    v2 Coords = MenuComponent->Coords;
    
    int StringLength = GetLength(TextBox->FontString.Text);
    
    v2 CharCoords[100];
    CharCoords[0] = v2(TextBox->TextCoords.x, TextBox->TextCoords.y);
    
    real32 ClosestX = TextBox->TextCoords.x;
    real32 CharX = TextBox->TextCoords.x;
    int Closest = 0;
    for (int i = 0; i < StringLength; i++) {
        CharX += TextBox->FontString.Advances[i];
        real32 DiffClosest = fabsf(ClosestX - MouseCoords.x);
        real32 DiffCurrent = fabsf(CharX - MouseCoords.x);
        if (DiffCurrent < DiffClosest) {
            ClosestX = CharX;
            Closest = i + 1;
        }
    }
    
    TextBox->CursorPosition = Closest;
}

internal void
MenuTextBoxAddChar(menu_component *MenuComponent, const char *Char)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        FontStringSetText(&TextBox->FontString, Insert(TextBox->FontString.Text, TextBox->CursorPosition, Char));
        IncrementCursorPosition(TextBox);
    }
}

internal void
MenuTextBoxRemoveChar(menu_component *MenuComponent)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->FontString.Length + 1 < TextBox->MaxTextLength) {
        DecrementCursorPosition(TextBox);
        FontStringSetText(&TextBox->FontString, RemoveAt(TextBox->FontString.Text, TextBox->CursorPosition));
    }
}

internal void
MenuTextBoxReplaceText(menu_component *MenuComponent, char *NewText)
{
    menu_textbox* TextBox = (menu_textbox*)MenuComponent->Data;
    if (TextBox->FontString.Length + GetLength(NewText) < TextBox->MaxTextLength) {
        FontStringSetText(&TextBox->FontString, Insert(TextBox->FontString.Text, TextBox->CursorPosition, NewText));
        TextBox->CursorPosition += GetLength(NewText);
    }
}

internal void
MenuResizeTextBox(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_textbox *TextBox = (menu_textbox*)MenuComponent->Data;
    FontStringResize(&TextBox->FontString, ResizeEquivalentAmount(MenuComponent->DefaultTextPixelHeight, ResizeFactors.y));
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddTextBox(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_textbox *TextBox, menu_component *Align)
{
    TextBox->FontString.Color = TextBox->CurrentTextColor;
    FontStringInit(&TextBox->FontString);
    
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = MenuComponent->Dim;
    MenuComponent->DefaultTextPixelHeight = TextBox->FontString.PixelHeight;
    MenuComponent->Type = menu_component_type::TextBox;
    MenuComponent->ID = ID;
    MenuComponent->AlignWith = Align;
    
    MenuComponent->Data = qalloc((void*)TextBox, sizeof(menu_textbox));
    MenuResizeTextBox(Menu, MenuComponent, 0);
    
    Menu->TextBoxes = MenuAddToComponentList(Menu->TextBoxes, MenuComponent);
    return MenuComponent;
}


// menu_logo
internal void
MenuResizeLogo(menu *Menu, menu_component *MenuComponent, v2 ResizeFactors)
{
    menu_logo *Logo = (menu_logo*)MenuComponent->Data;
    MenuComponent->Dim = ResizeEquivalentAmount(MenuComponent->DefaultDim, ResizeFactors.y);
    ResizeTexture(Logo->Tex, MenuComponent->Dim);
    MenuComponent->PaddingDim = MenuComponent->Dim + (Menu->Padding * 2);
}

internal menu_component*
MenuAddLogo(menu *Menu, int ID, v2 GridCoords, v2 Dim, menu_logo *Logo)
{
    menu_component *MenuComponent = MenuGetNextComponent(Menu);
    
    MenuComponent->GridCoords = GridCoords;
    MenuComponent->Dim = Dim;
    MenuComponent->DefaultDim = Dim;
    MenuComponent->Type = menu_component_type::Logo;
    MenuComponent->ID = ID;
    
    MenuComponent->Data = qalloc((void*)Logo, sizeof(menu_logo));
    MenuResizeLogo(Menu, MenuComponent, 0);
    
    Menu->Logos = MenuAddToComponentList(Menu->Logos, MenuComponent);
    return MenuComponent;
}
inline menu_component* MenuAddLogo(menu *Menu, v2 GridCoords, v2 Dim, menu_logo *Logo)
{
    return MenuAddLogo(Menu, -1, GridCoords, Dim, Logo);
}

// menu
internal void
MenuSortActiveComponents(menu *Menu)
{
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *C = &Menu->Components[i];
        if (C->Type == menu_component_type::Button || C->Type == menu_component_type::TextBox) {
            Menu->ActiveComponents[Menu->NumOfActiveComponents++] = C;
        }
    }
    
    // X-Sort using Insertion Sort
    {
        int i = 1;
        while (i < Menu->NumOfActiveComponents) {
            int j = i;
            while (j > 0 && Menu->ActiveComponents[j-1]->GridCoords.x > Menu->ActiveComponents[j]->GridCoords.x) {
                menu_component *Temp = Menu->ActiveComponents[j];
                Menu->ActiveComponents[j] = Menu->ActiveComponents[j-1];
                Menu->ActiveComponents[j-1] = Temp;
                j = j - 1;
            }
            i++;
        }
    }
    
    // Y-Sort using Insertion Sort
    {
        int i = 1;
        while (i < Menu->NumOfActiveComponents) {
            int j = i;
            while (j > 0 && Menu->ActiveComponents[j-1]->GridCoords.y > Menu->ActiveComponents[j]->GridCoords.y) {
                menu_component *Temp = Menu->ActiveComponents[j];
                Menu->ActiveComponents[j] = Menu->ActiveComponents[j-1];
                Menu->ActiveComponents[j-1] = Temp;
                j = j - 1;
            }
            i++;
        }
    }
}

internal void
MenuInit(menu *Menu, v2 DefaultDim, int Padding)
{
    Menu->DefaultDim = DefaultDim;
    Menu->DefaultPadding = Padding;
    Menu->NumOfComponents = 0;
    Menu->MaxNumOfComponents = ArrayCount(Menu->Components);
    Menu->Initialized = true;
}

internal void
MenuReset(menu *Menu) 
{
    for (int i = 0; i < Menu->NumOfComponents; i++)
        Menu->Components[i].Active = false;
    
    Menu->ActiveIndex = 0;
    Menu->Reset = false;
}

internal menu_events*
HandleMenuEvents(menu *Menu, platform_input *Input)
{
    UpdateInputInfo(Input);
    
    //printf("%d\n", (int)Input->PreviousInputInfo.InputMode);
    
    bool KeyboardMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Keyboard;
    bool KeyboardPreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Keyboard;
    
    bool ControllerMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Controller;
    bool ControllerPreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Controller;
    
    bool MouseMode = Input->CurrentInputInfo.InputMode == platform_input_mode::Mouse;
    bool MousePreviousMode = Input->PreviousInputInfo.InputMode == platform_input_mode::Mouse;
    
    Menu->Events.ButtonClicked = -1;
    
    v2 MouseCoords = v2(Input->MouseX, Input->MouseY);
    
    if (KeyboardMode || ControllerMode) {
        if (KeyboardPreviousMode || ControllerPreviousMode) {
            platform_controller_input *Controller = 0;
            if (KeyboardMode)
                Controller = Input->CurrentInputInfo.Controller;
            else if (ControllerMode)
                Controller = Input->CurrentInputInfo.Controller;
            
            if (KeyDown(&Controller->MoveUp)) {
                DecrActive(Menu);
            }
            if (KeyDown(&Controller->MoveDown)) {
                IncrActive(Menu);
            }
            if(KeyDown(&Input->Keyboard.Tab)) {
                IncrActive(Menu);
            }
            if (KeyDown(&Controller->Enter)) {
                menu_component *C = Menu->ActiveComponents[Menu->ActiveIndex];
                if (C->Type == menu_component_type::Button)
                    Menu->Events.ButtonClicked = C->ID;
            }
        }
        
        Menu->ActiveComponents[Menu->ActiveIndex]->Active = true;
    }
    else if (MouseMode) {
        if (Menu->ActiveComponents[Menu->ActiveIndex]->Type == menu_component_type::Button)
            Menu->ActiveComponents[Menu->ActiveIndex]->Active = false;
        
        if (KeyDown(&Input->MouseButtons[0])) {
            Menu->Events.ButtonClicked = MenuButtonClicked(Menu, MouseCoords);
            Menu->Events.TextBoxClicked = MenuTextBoxClicked(Menu, MouseCoords);
        }
        
        if (MenuButtonHovered(Menu, v2(Input->MouseX, Input->MouseY)))
            PlatformSetCursorMode(Input, platform_cursor_mode::Hand);
        else
            PlatformSetCursorMode(Input, platform_cursor_mode::Arrow);
    }
    
    //menu_component *ActiveTextBox = MenuTextBoxGetActive(Menu->TextBoxes);
    menu_component *ActiveTextBox = Menu->ActiveComponents[Menu->ActiveIndex];
    if (ActiveTextBox->Type == menu_component_type::TextBox)
    {
        for (int i = 0; i < 10; i++) {
            if (KeyDown(&Input->Keyboard.Numbers[i]))
                MenuTextBoxAddChar(ActiveTextBox, IntToString(i));
        }
        
        if (KeyPressed(&Input->MouseButtons[0])) {
            MenuTextBoxMouseMoveCursor(ActiveTextBox, MouseCoords);
        }
        
        if (KeyDown(&Input->Keyboard.Left))
            MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Left);
        if (KeyDown(&Input->Keyboard.Right))
            MenuTextBoxArrowKeysMoveCursor(ActiveTextBox, menu_direction::Right);
        if (KeyDown(&Input->Keyboard.CtrlV))
            MenuTextBoxReplaceText(ActiveTextBox, Input->Keyboard.Clipboard);
        
        if (KeyPressed(&Input->Keyboard.Backspace, Input))
            MenuTextBoxRemoveChar(ActiveTextBox);
        if (KeyDown(&Input->Keyboard.Period))
            MenuTextBoxAddChar(ActiveTextBox, ".");
    }
    
    return &Menu->Events;
}

internal void
UpdateMenu(menu *Menu, v2 BufferDim)
{
    if (Menu->ScreenDim != BufferDim) {
        Menu->ScreenDim = BufferDim;
        
        v2 ResizeFactors = GetResizeFactor(Menu->DefaultDim, BufferDim);
        Menu->Padding = (int)ResizeEquivalentAmount((real32)Menu->DefaultPadding, ResizeFactors.y);
        
        for (int i = 0; i < Menu->NumOfComponents; i++) {
            menu_component *MenuComponent = &Menu->Components[i];
            if (MenuComponent->Type == menu_component_type::Button)
                MenuResizeButton(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::Text)
                MenuResizeText(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::TextBox)
                MenuResizeTextBox(Menu, MenuComponent, ResizeFactors);
            if (MenuComponent->Type == menu_component_type::Logo)
                MenuResizeLogo(Menu, MenuComponent, ResizeFactors);
        }
        
        // Setting up rows
        memset(Menu->Rows, 0, sizeof(menu_grid_row) * 10);
        for (int i = 0; i < Menu->NumOfComponents; i++) {
            menu_component *C = &Menu->Components[i];
            menu_grid_row* R = &Menu->Rows[(int)C->GridCoords.y];
            
            real32 Width = 0;
            if (C->AlignWith == 0) 
                Width = C->PaddingDim.x;
            else if (C->AlignWith != 0)
                Width = C->AlignWith->PaddingDim.x;
            
            R->MenuGridColumnWidth[(int)C->GridCoords.x] = Width;
            R->Dim.x += Width;
            if (R->Dim.y < C->PaddingDim.y)
                R->Dim.y = C->PaddingDim.y;
        }
        
        // Height of GUI
        Menu->Dim.y = 0;
        for (int i = 0; i < 10; i++)
            Menu->Dim.y += Menu->Rows[i].Dim.y;
        
        Menu->Dim.x = 0;
        for (int i = 0; i <  Menu->NumOfComponents; i++) {
            menu_component *C = &Menu->Components[i];
            menu_grid_row *R = &Menu->Rows[(int)C->GridCoords.y];
            
            // Calculate column location to center
            C->Coords.x = (-R->Dim.x)/2;
            for (int  i = 0; i < C->GridCoords.x; i++)
                C->Coords.x += Menu->Rows[(int)C->GridCoords.y].MenuGridColumnWidth[i];
            
            // Find biggest row
            if (Menu->Dim.x < R->Dim.x) {
                Menu->Dim.x = R->Dim.x;
                Menu->Coords.x = C->Coords.x - (Menu->Padding);
            }
            // Calculate row location to center
            C->Coords.y = ((-Menu->Dim.y)/2) + ((R->Dim.y - C->PaddingDim.y)/2);
            for (int i = 0; i < C->GridCoords.y; i++) {
                menu_grid_row* tempR = &Menu->Rows[i];
                C->Coords.y += tempR->Dim.y;
            }
        }
        
        menu_component* C = &Menu->Components[0];
        Menu->Coords.y = C->Coords.y - (Menu->Padding);
    }
}

internal void
DrawMenu(menu *Menu, real32 Z)
{
    for (int i = 0; i < Menu->NumOfComponents; i++) {
        menu_component *MenuComponent = &Menu->Components[i];
        
        v2 Padding = (MenuComponent->Dim - MenuComponent->PaddingDim)/2;
        
        if (MenuComponent->Type == menu_component_type::Button) {
            menu_button *Button = (menu_button*)MenuComponent->Data;
            
            if (MenuComponent->Active == true) {
                Button->CurrentColor = Button->HoverColor;
                Button->FontString.Color = Button->HoverTextColor;
            }
            else {
                Button->CurrentColor = Button->DefaultColor;
                Button->FontString.Color = Button->DefaultTextColor;
            }
            v2 SDim = FontStringGetDim(&Button->FontString);
            v2 TextCoords = MenuComponent->Coords + ((MenuComponent->Dim - SDim)/2);
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, Z), MenuComponent->Dim, Button->CurrentColor, 0.0f);
            FontStringPrint(&Button->FontString, TextCoords - Padding);
        }
        else if (MenuComponent->Type == menu_component_type::TextBox) {
            menu_textbox *TextBox = (menu_textbox*)MenuComponent->Data;
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, 50.0f), MenuComponent->Dim, TextBox->CurrentColor, 0.0f);
            
            if (MenuComponent->Active) {
                v2 SDim = FontStringGetDim(&TextBox->FontString);
                TextBox->TextCoords = MenuComponent->Coords;
                TextBox->TextCoords.y += (MenuComponent->Dim.y - SDim.y)/2;
                real32 CursorPadding = (MenuComponent->Dim.y)/4;
                real32 CursorX = 0;
                real32 LeftX = MenuComponent->Coords.x;
                real32 RightX = MenuComponent->Coords.x + MenuComponent->Dim.x;
                
                if (TextBox->DisplayRight == 0) {
                    TextBox->DisplayRight = MenuComponent->Dim.x;
                    TextBox->DisplayLeft = 0;
                }
                
                for (int i = 0; i <  TextBox->CursorPosition; i++)
                    CursorX += TextBox->FontString.Advances[i];
                
                if (TextBox->DisplayRight < CursorX + CursorPadding) {
                    TextBox->DisplayLeft = TextBox->DisplayLeft + (CursorX - TextBox->DisplayRight);
                    TextBox->DisplayRight = CursorX;
                    TextBox->PaddingRight = true;
                }
                if (TextBox->DisplayLeft > CursorX - CursorPadding) {
                    TextBox->DisplayRight = TextBox->DisplayRight - (TextBox->DisplayLeft - CursorX);
                    TextBox->DisplayLeft = CursorX;
                    TextBox->PaddingRight = false;
                }
                
                
                if (!TextBox->PaddingRight) {
                    TextBox->TextCoords.x +=  CursorPadding;
                }
                else if (TextBox->PaddingRight) {
                    TextBox->TextCoords.x -=  CursorPadding;
                }
                
                
                TextBox->TextCoords.x -= TextBox->DisplayLeft;
                CursorX += TextBox->TextCoords.x;
                Push(RenderGroup, v3(CursorX - Padding.x, MenuComponent->Coords.y - Padding.y, 100.0f), v2(5.0f, MenuComponent->Dim.y), 0xFF000000, 0.0f);
            }
            
            FontStringPrint(&TextBox->FontString, TextBox->TextCoords - Padding, MenuComponent->Coords - Padding, MenuComponent->Dim);
        }
        else if (MenuComponent->Type == menu_component_type::Text) {
            menu_text *Text = (menu_text*)MenuComponent->Data;
            v2 TextCoords = MenuComponent->Coords;
            if (MenuComponent->AlignWith != 0)
                TextCoords.x = MenuComponent->Coords.x + MenuComponent->AlignWith->Dim.x - MenuComponent->Dim.x;
            FontStringPrint(&Text->FontString, TextCoords - Padding);
        }
        else if (MenuComponent->Type == menu_component_type::Logo) {
            menu_logo *Logo = (menu_logo*)MenuComponent->Data;
            Push(RenderGroup, v3(MenuComponent->Coords - Padding, 100.0f), MenuComponent->Dim, 
                 Logo->Tex, 0.0f, BlendMode::gl_src_alpha);
        }
    }
    
    Push(RenderGroup, v3(Menu->Coords, 50.0f), Menu->Dim + (Menu->Padding * 2), Menu->BackgroundColor, 0.0f);
}
inline void DrawMenu(menu *Menu) { DrawMenu(Menu, 0); }

#endif //MENU_H