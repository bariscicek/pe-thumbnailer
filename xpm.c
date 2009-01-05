#include <png.h>

#include "thumbnailer.h"
#include "xpm.h"

/*
 * Code taken from wine/programs/winemenubuilder/winemenubuilder.c
 */


#define MASK(x,y) (pAND[(x) / 8 + (nHeight - (y) - 1) * nANDWidthBytes] & (1 << (7 - (x) % 8)))
#define COLOR(x,y) (b8BitColors ? pXOR[(x) + (nHeight - (y) - 1) * nXORWidthBytes] : (x) % 2 ? pXOR[(x) / 2 + (nHeight - (y) - 1) * nXORWidthBytes] & 0xF : (pXOR[(x) / 2 + (nHeight - (y) - 1) * nXORWidthBytes] & 0xF0) >> 4)


gboolean
xpm_save_icon_file (const BITMAPINFO *pIcon, const gchar *szXPMFileName)
{
    FILE *fXPMFile;
    
    gint nHeight;
    gint nXORWidthBytes;
    gint nANDWidthBytes;
    gboolean b8BitColors;
    gint nColors;
    const guchar *pXOR;
    const guchar *pAND;
    gboolean aColorUsed[256] = {0};
    gint nColorsUsed = 0;
    gint i,j;

    if (!((pIcon->bmiHeader.biBitCount == 4) || (pIcon->bmiHeader.biBitCount == 8)))
    {
        g_warning ("Unsupported color depth %d-bit\n", pIcon->bmiHeader.biBitCount);
        return FALSE;
    }

    if (!(fXPMFile = g_fopen(szXPMFileName, "w")))
    {
        g_warning ("unable to open '%s' for writing: %s\n", szXPMFileName, g_strerror(errno));
        return FALSE;
    }

    nHeight = pIcon->bmiHeader.biHeight / 2;
    nXORWidthBytes = 4 * ((pIcon->bmiHeader.biWidth * pIcon->bmiHeader.biBitCount / 32)
                          + ((pIcon->bmiHeader.biWidth * pIcon->bmiHeader.biBitCount % 32) > 0));
    nANDWidthBytes = 4 * ((pIcon->bmiHeader.biWidth / 32)
                          + ((pIcon->bmiHeader.biWidth % 32) > 0));
    b8BitColors = pIcon->bmiHeader.biBitCount == 8;
    nColors = pIcon->bmiHeader.biClrUsed ? pIcon->bmiHeader.biClrUsed
        : 1 << pIcon->bmiHeader.biBitCount;
    pXOR = (const guchar*) pIcon + sizeof (BITMAPINFOHEADER) + (nColors * sizeof (RGBQUAD));
    pAND = pXOR + nHeight * nXORWidthBytes;

    for (i = 0; i < nHeight; i++) {
        for (j = 0; j < pIcon->bmiHeader.biWidth; j++) {
            if (!aColorUsed[COLOR(j,i)] && !MASK(j,i))
            {
                aColorUsed[COLOR(j,i)] = TRUE;
                nColorsUsed++;
            }
        }
    }

    if (g_fprintf(fXPMFile, "/* XPM */\n/* nautilus pe file thumbnail */\nstatic char *icon[] = {\n") <= 0)
        goto error;
    if (g_fprintf(fXPMFile, "\"%d %d %d %d\",\n",
                (gint) pIcon->bmiHeader.biWidth, nHeight, nColorsUsed + 1, 2) <=0)
        goto error;

    for (i = 0; i < nColors; i++) {
        if (aColorUsed[i])
            if (fprintf(fXPMFile, "\"%.2X c #%.2X%.2X%.2X\",\n", i, pIcon->bmiColors[i].rgbRed,
                        pIcon->bmiColors[i].rgbGreen, pIcon->bmiColors[i].rgbBlue) <= 0)
                goto error;
    }
    if (g_fprintf(fXPMFile, "\"   c None\"") <= 0)
        goto error;

    for (i = 0; i < nHeight; i++)
    {
        if (g_fprintf(fXPMFile, ",\n\"") <= 0)
            goto error;
        for (j = 0; j < pIcon->bmiHeader.biWidth; j++)
        {
            if MASK(j,i)
                {
                    if (g_fprintf(fXPMFile, "  ") <= 0)
                        goto error;
                }
            else
                if (g_fprintf(fXPMFile, "%.2X", COLOR(j,i)) <= 0)
                    goto error;
        }
        if (g_fprintf(fXPMFile, "\"") <= 0)
            goto error;
    }
    if (g_fprintf(fXPMFile, "};\n") <= 0)
        goto error;


    fclose(fXPMFile);
    return TRUE;

 error: 
    fclose(fXPMFile);
    g_unlink( szXPMFileName );
    return FALSE;
}

gboolean
png_save_icon_file_direct (const BITMAPINFO *pIcon, gint size, const char *png_filename)
{
    FILE *fp;
    gchar *data;
    gint i;
    
    if (! (fp = g_fopen (png_filename, "w")))
    {
        g_warning ("unable to open '%s' for writing: %s\n", png_filename, g_strerror(errno));
        return FALSE;       
    }
    
    data = g_malloc0 (size);
    
    memcpy (data, pIcon, size);
    
    if (fwrite (data, sizeof (gchar), size, fp) > 0)
    {
        fclose (fp);
        return TRUE;
    }
    else
    {
        g_warning ("Error occured.");
    }
    fclose (fp);
    return FALSE;
    
}

gboolean
png_save_icon_file (const BITMAPINFO *pIcon, const char *png_filename)
{
    static const char comment_key[] = "Created from";
    FILE *fp;
    png_structp png_ptr;
    png_color *nPalette;
    png_infop info_ptr;
    png_text comment;
    gint nXORWidthBytes, nANDWidthBytes, color_type = 0, i, j;
    guchar *row, *copy = NULL;
    const guchar *pXOR, *pAND = NULL;
    gint nWidth  = pIcon->bmiHeader.biWidth;
    gint nHeight = pIcon->bmiHeader.biHeight;
    gint nBpp    = pIcon->bmiHeader.biBitCount;

    switch (nBpp)
    {
    case 32:
        color_type |= PNG_COLOR_MASK_ALPHA;
        /* fall through */
    case 24:
        color_type |= PNG_COLOR_MASK_COLOR;
        break;
    case 8:
        color_type |= PNG_COLOR_TYPE_PALETTE;
        color_type |= PNG_COLOR_MASK_COLOR;
        break;
    default:
        break;
            
    }

    if (!(fp = g_fopen(png_filename, "w")))
    {
        g_warning ("unable to open '%s' for writing: %s\n", png_filename, g_strerror(errno));
        return FALSE;
    }
        
    nXORWidthBytes = 4 * ((nWidth * nBpp + 31) / 32);
    nANDWidthBytes = 4 * ((nWidth + 31) / 32);
    pXOR = (const guchar*) pIcon + sizeof(BITMAPINFOHEADER) + pIcon->bmiHeader.biClrUsed * sizeof(RGBQUAD);
    if (pIcon->bmiHeader.biClrUsed == 0 && nBpp == 8 || nBpp == 4)
    {
        gint nClrCount; 
        
        nClrCount = 1 << nBpp;
        /* Read palette from bmiColors and change pXOR location accordingly */
        nPalette = (png_color *) g_malloc0 (nClrCount * (sizeof (png_color)));
        for (i=0; i < nClrCount; i++)
        {  
            (nPalette + i)->red = ((RGBQUAD *) pXOR + i)->rgbRed;
            (nPalette + i)->green = ((RGBQUAD *) pXOR + i)->rgbGreen;
            (nPalette + i)->blue = ((RGBQUAD *) pXOR + i)->rgbBlue;
        }
        
        pXOR += nClrCount * sizeof (RGBQUAD);
    }
    if (nHeight > nWidth)
    {
        nHeight /= 2;
        pAND = pXOR + nHeight * nXORWidthBytes;
    }

    /* Apply mask if present */
    if (pAND)
    {
        RGBQUAD bgColor;

        /* copy bytes before modifying them */
        copy = g_malloc0 ( nHeight * nXORWidthBytes );
        memcpy( copy, pXOR, nHeight * nXORWidthBytes );
        pXOR = copy;

        /* image and mask are upside down reversed */
        row = copy + (nHeight - 1) * nXORWidthBytes;

        /* top left corner */
        bgColor.rgbRed   = row[0];
        bgColor.rgbGreen = row[1];
        bgColor.rgbBlue  = row[2];
        bgColor.rgbReserved = 0;

        for (i = 0; i < nHeight; i++, row -= nXORWidthBytes)
            for (j = 0; j < nWidth; j++, row += nBpp >> 3)
                if (MASK(j, i))
                {
                    RGBQUAD *pixel = (RGBQUAD *)row;
                    pixel->rgbBlue  = bgColor.rgbBlue;
                    pixel->rgbGreen = bgColor.rgbGreen;
                    pixel->rgbRed   = bgColor.rgbRed;
                    if (nBpp == 32)
                        pixel->rgbReserved = bgColor.rgbReserved;
                }
    }

    comment.text = NULL;

    if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) ||
        !(info_ptr = png_create_info_struct(png_ptr)))
        goto error;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* All future errors jump here */
        g_warning ("Error happened\n");
        goto error;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, nWidth, nHeight, (nBpp < 8 ? nBpp : 8),
                  color_type,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    if (nBpp == 8 || nBpp == 4)
    {
        png_set_PLTE (png_ptr, info_ptr, nPalette, pIcon->bmiHeader.biClrUsed ? pIcon->bmiHeader.biClrUsed : 1 << nBpp);
    }
    
                  

    /* Set comment */
    comment.compression = PNG_TEXT_COMPRESSION_NONE;
    comment.key = (png_charp)comment_key;
    comment.text = g_strdup ("Pe Thumbnailer");
    comment.text_length = strlen (comment.text);
    png_set_text(png_ptr, info_ptr, &comment, 1);


    png_write_info(png_ptr, info_ptr);
    png_set_bgr(png_ptr);
    for (i = nHeight - 1; i >= 0 ; i--)
        png_write_row(png_ptr, (png_bytep)pXOR + nXORWidthBytes * i);
    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    if (png_ptr) png_destroy_write_struct(&png_ptr, NULL);
    fclose(fp);
    g_free(copy);
    g_free (comment.text);
    return TRUE;

 error:
    if (png_ptr) png_destroy_write_struct(&png_ptr, NULL);
    fclose(fp);
    unlink(png_filename);
    g_free (copy);
    g_free (comment.text);
    return FALSE;
}

