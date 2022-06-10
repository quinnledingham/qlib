internal Image
LoadImage(const char* FileName)
{
    Image NewImage;
    NewImage.data = stbi_load(FileName, &NewImage.x, &NewImage.y, &NewImage.n, 4);
    NewImage.n = 4;
    Assert(NewImage.data != 0); // Image loaded
    
    return NewImage;
}

internal Image
ResizeImage(Image ToResize, int Width, int Height)
{
    Image ResizedImage = {};
    ResizedImage.data = (unsigned char*)malloc(Width * Height * ToResize.n);
    stbir_resize_uint8(ToResize.data, ToResize.x, ToResize.y, 0, ResizedImage.data, Width, Height, 0, ToResize.n);
    
    ResizedImage.x = Width;
    ResizedImage.y = Height;
    ResizedImage.n = ToResize.n;
    
    stbi_image_free(ToResize.data);
    
    return ResizedImage;
}

internal Texture
LoadTexture(const char* FileName, int Width, int Height)
{
    Image Temp;
    Texture Return;
    Temp = LoadImage(FileName);
    Temp = ResizeImage(Temp, Width, Height);
    Return.Init(&Temp);
    
    return Return;
}

internal void
SaveImage(Image* image, const char* SaveFileName)
{
    Strinq FullDir = S() + "imagesaves/" + SaveFileName;
    FILE *NewFile = fopen(GetData(FullDir), "w");
    
    // Change filename period to underscore
    char *f = (char*)qalloc(100);
    CopyBuffer(f, SaveFileName, Length(SaveFileName));
    int i = 0;
    while(f[i] != 0)
    {
        if (f[i] == '.')
        {
            f[i] = '_';
        }
        i++;
    }
    
    
    // Header File
    fprintf(NewFile,
            "int %sx = %d;\n"
            "int %sy = %d;\n"
            "int %sn = %d;\n"
            "static unsigned char %s[%d] = \n"
            "{\n",
            f,
            image->x,
            f,
            image->y,
            f,
            image->n,
            f,
            (image->x * image->y * image->n));
    
    unsigned char* imgtosave = image->data;
    for(int i = 0; i < (image->x * image->y * image->n); i++)
    {
        fprintf(NewFile, "0x%x ,", *imgtosave);
        *imgtosave++;
    }
    fprintf(NewFile,  "\n};\n");
    fclose(NewFile);
    
    // Cpp File
    char* filenamecpp = (char*)qalloc(GetData(FullDir), Length(FullDir));
    
    int j = 0;
    int extension = 0;
    char* cursor = FullDir.Data;
    while (!extension)
    {
        if (*cursor == '.')
        {
            filenamecpp[j] = *cursor;
            extension = 1;
        }
        else
        {
            filenamecpp[j] = *cursor;
        }
        
        cursor++;
        j++;
    }
    filenamecpp[j] = 'c';
    filenamecpp[j + 1] = 'p';
    filenamecpp[j + 2] = 'p';
    filenamecpp[j + 3] = 0;
    
    FILE *newcppfile = fopen(filenamecpp, "w");
    fprintf(newcppfile, 
            "internal void\n"
            "LoadImageFrom%s(Image* image)\n"
            "{\n"
            "image->data = (unsigned char *)qalloc((void*)&%s, sizeof(%s));\n"
            "image->x = %sx;\n"
            "image->y = %sy;\n"
            "image->n = %sn;\n"
            "}\n"
            ,f
            ,f
            ,f
            ,f
            ,f
            ,f
            );
    
    fclose(newcppfile);
}