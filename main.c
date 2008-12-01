
#include <gtk/gtk.h>
#include <glib/goption.h>
#include <glib-2.0/glib/gscanner.h>
#include <glib-2.0/glib/gfileutils.h>
#include <gio/gio.h>
#include <glib-2.0/glib/gmappedfile.h>

#include "thumbnailer.h"

static goffset map_total_size;
static GMappedFile *mapped_file;
static gchar *map_base;

typedef struct _coff_header_struct coff_header;
typedef struct _optional_header_std_struct optional_header_std;
typedef struct _optional_header_nt_struct optional_header_nt;
typedef struct _IMAGE_DATA_DIRECTORY image_data_directory;
typedef struct _section_table_struct    section_table;
typedef struct _resource_directory_table_struct resource_directory_table;
typedef struct _resource_directory_entries_struct resource_directory_entries;
typedef struct _resource_directory_string_struct resource_directory_string;
typedef struct _resource_data_entry_struct  resource_data_entry;
typedef struct _resource_directory_entry_row_struct resource_directory_entry_row;

void    dump_directory_table (goffset root_rva,
        GInputStream *fis,
        GError *error);





goffset
dump_section (section_table *table, gint idx, GInputStream *fis, GDataInputStream *dis, GError *error)
{
    goffset offset;
    int i;
    
    offset = table[idx].pointertorawdata;
    g_seekable_seek (G_SEEKABLE (fis), offset, G_SEEK_SET, NULL, &error);

    for (i=0; i < table[idx].sizeofrawdata; i++)
    {
        if (!(i %8) && (i % 16))
            printf ("- ");
        if (!(i % 16))
            printf ("\n%.8X\t", offset);
        printf ("%.2x ", g_data_input_stream_read_byte (dis, NULL, &error));
        
    }
    
    return offset;
    

}

goffset
convert_to_offset  (image_data_directory head,
                    section_table   *tables,
                    gint numberofsections)
{    

    int i;
    if (head.rva == 0) return 0;
    
    for (i = numberofsections -1; i >= 0; i--)
    {
        if (tables[i].virtualaddress <= head.rva &&
                head.rva + head.size <= tables[i].virtualaddress + tables[i].sizeofrawdata)
        {
            return (goffset) map_base + tables[i].pointertorawdata + head.rva - tables[i].virtualaddress;
        }

    }
}

const gchar *
get_resource_type (gint id)
{
    gchar *string = NULL;
    
    static gchar *const types[] =
    {
        NULL,
        "CURSOR",
        "BITMAP",
        "ICON",
        "MENU",
        "DIALOG",
        "STRING",
        "FONTDIR",
        "FONT",
        "ACCELERATOR",
        "RCDATA",
        "MESSAGETABLE",
        "GROUP_CURSOR",
        NULL,
        "GROUP_ICON",
        NULL,
        "VERSION",
        "DLGINCLUDE",
        NULL,
        "PLUGPLAY",
        "VXD",
        "ANICURSOR",
        "ANIICON",
        "HTML",
        "RT_MANIFEST"
    };

    if ((size_t)id < sizeof(types)/sizeof(types[0])) return types[id];
    string = g_malloc (16 * sizeof (gchar));
    g_sprintf (string, "0x%x", id);
    return  string;
}


gchar *
get_resource_string (goffset base_rva,
        goffset    string_offset,
        GInputStream    *fis)
{
    GError  *error = NULL;
    goffset orig_offset;
    gint readcount;
    gint16 length;
    gchar   *string;
    
    orig_offset = g_seekable_tell (G_SEEKABLE (fis));
    g_seekable_seek (G_SEEKABLE (fis), base_rva + string_offset, G_SEEK_SET, NULL, &error);
    g_assert (error == NULL);
    
    readcount = g_input_stream_read (fis, &length, sizeof (length), NULL, &error);
    if (readcount > 0 && error == NULL)
    {
        string = g_malloc0 (2 * (length + 1) * sizeof (gchar));
        readcount = g_input_stream_read (fis, string, 2 * length, NULL, &error);
        if (readcount > 0 && error == NULL)
        {
            g_seekable_seek (G_SEEKABLE (fis), orig_offset, G_SEEK_SET, NULL, &error);
            string = g_convert (string, 2 * length, "utf-8", "ucs-2", NULL, NULL, &error);
            return string;
        }        
    }
    
    return NULL;
}


int
main (int argc, gchar **argv)
{
    gboolean is_verbose;

    GOptionContext  *option_context;
    GOptionEntry    option_entries[] = { 
        { "verbose",'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &is_verbose, "Prints verbose output and debug strings", NULL },
        { NULL }
    };
    
    GInputStream *fis;
    GDataInputStream *dis;
    GFile   *pe_file;
    const gchar *file_name;
    GError   *error = NULL;
    gboolean in_coff = FALSE;
    
    GFileInfo   *file_info;
    coff_header my_coff_header;
    optional_header_std my_optional_header_std;
    optional_header_nt my_optional_header_nt;
    image_data_directory    *image_data_directories;
    image_data_directory    rsrc_directory;
    section_table   *section_tables;
    section_table   rsrc_table;
    goffset offset;
    gssize  readcount;
    gint i,j,k;
    gint sectioncount;
    
    gtk_init (&argc, &argv);
    option_context = g_option_context_new ("- gets icons from PE file format");
    g_option_context_set_summary (option_context,
            "Reads COFF from PE file and extracts image icon to be used as "
            "thumbnailer for Nautilus and similar file managers.");
    
    g_option_context_set_description(option_context, NULL);
    g_option_context_add_main_entries (option_context, option_entries, NULL);
    
    if (!g_option_context_parse (option_context, &argc, &argv, &error))
    {
        fprintf (stderr, "A fatal error occured while parsing options:", error->message);
        return 1;
    }
    
    if (!argv[1])
    {
        printf ("%s", g_option_context_get_help (option_context, TRUE, NULL));
        return 1;
    }
    
    pe_file = g_file_new_for_commandline_arg (argv[1]);
    fis = (GInputStream *) g_file_read (pe_file, NULL, &error);
    g_assert (error == NULL);
    
    file_info = g_file_input_stream_query_info ((GFileInputStream *) fis, "standard", NULL, &error);
    g_assert (error == NULL);
    
    map_total_size = g_file_info_get_size (file_info);
    g_warning ("Total size: %d", map_total_size);
    
    file_name = g_file_get_parse_name (pe_file);
    mapped_file = g_mapped_file_new (file_name, FALSE, &error);
    g_assert (error == NULL);
   
    map_base = g_mapped_file_get_contents(mapped_file);
    dis = (GDataInputStream *) g_data_input_stream_new (fis);

    /*
     * Get COFF file header
     */
    
    while (error == NULL)
    {
        gint32 fourbyte; 
        
        fourbyte = g_data_input_stream_read_int32 (dis, NULL, &error);
        if (fourbyte == 0x50450000)
        {
            offset = g_seekable_tell (G_SEEKABLE(fis));
            g_warning ("Header found before: %x", offset-4);
/*
            map_base += offset - 4;
*/
            in_coff = TRUE;
            break;
        }
    }
    
    if (!in_coff)
    {
        fprintf (stderr, "Could not find COFF header: not a PE file.");
        return 2;
    }
    
    /*
     * Check if PE file has .rsrc header
     */   
    readcount = g_input_stream_read (fis, &my_coff_header, sizeof (my_coff_header), NULL, &error);
    printf ("\nCOFF Header:\n");
    if (readcount > 0 && error == NULL)
    {
        printf ("Machine code: %x\n"
                "Number of sections: %x\n"
                "Time and Date stamp: %x\n"
                "Pointer to symbol table: %x\n"
                "Number of symbols: %x\n"
                "Size of optional header: %x\n"
                "Characteristics: %x\n", 
                my_coff_header.machine, 
                my_coff_header.numberofsections, 
                my_coff_header.timedatestamp,
                my_coff_header.pointertosymboltable,
                my_coff_header.numberofsymbols,
                my_coff_header.sizeofoptionalheader,
                my_coff_header.characteristics);
        if (my_coff_header.characteristics & IMAGE_FILE_32BIT_MACHINE)
            printf ("32BIT_MACHINE\n");
    }
    else
    {
        printf ("Couldn't read anything with this struct.");
        return 1;
    }
    
    printf ("\n\nOptional Standard headers:\n");
    readcount = g_input_stream_read (fis, &my_optional_header_std, sizeof (my_optional_header_std), NULL, &error);
    if (readcount > 0 && error == NULL)
    {
        printf ("Magic: %x\n"
                "Address of entry point: %x\n", 
                my_optional_header_std.magic,
                my_optional_header_std.addressofentrypoint);
    }
    else
    {
        printf ("No optional header, are you sure this is an image file.");
        return 1;
    }
    
    printf ("\n\nOptional NT Specific Headers:\n");
    readcount = g_input_stream_read (fis, &my_optional_header_nt, sizeof (my_optional_header_nt), NULL, &error);
    if (readcount > 0 && error == NULL)
    {
        printf ("Base address: %x\n"
                "Size of image: %x\n"
                "Size of headers: %x\n"
                "Checksum: %x\n"
                "Subsystem: %x\n"
                "Number of rva and sizes: %x\n",
                my_optional_header_nt.imagebase,
                my_optional_header_nt.sizeofimage,
                my_optional_header_nt.sizeofheaders,
                my_optional_header_nt.checksum,
                my_optional_header_nt.subsystem,
                my_optional_header_nt.numberofrvaandsizes);
    }
    else
    {
        printf ("No NT specific standar header, are you sure this is an NT file.");
        return 1;
    }
    

/*
    map_base += sizeof (coff_header) + my_coff_header.sizeofoptionalheader;
*/

    sectioncount = my_optional_header_nt.numberofrvaandsizes;
    image_data_directories = (image_data_directory *) g_malloc0 (sectioncount * sizeof (image_data_directory));
    section_tables = (section_table *) g_malloc0 (my_coff_header.numberofsections * sizeof (section_table));
    
    readcount = g_input_stream_read (fis, image_data_directories, sectioncount * sizeof (image_data_directory), NULL, &error);
    if (readcount > 0 && error == NULL)
    {              
        for (i = 0; i < sectioncount; i++)
        {
            printf ("Directory #%d rva: %8.x\t\tsize:%8.x\n",
                        i,
                        image_data_directories[i].rva,
                        image_data_directories[i].size);
        }
    }
    
    /*
     * Get the first section right after data directories
     */
    
    readcount = g_input_stream_read (fis, section_tables, my_coff_header.numberofsections * sizeof (section_table), NULL, &error);
    if (readcount > 0 && error == NULL)
    {
        for (i = 0; i < my_coff_header.numberofsections; i++)
        {
            gchar section_name[6];
            g_printf ("Section #%d, name: %c%c%c%c%c, virtualaddress: %x, sizeofrawdata: %x\n", i, 
                    section_tables[i].name, section_tables[i].name >> 8, section_tables[i].name >> 16, section_tables[i].name >> 24 , section_tables[i].name >> 32, section_tables[i].virtualaddress, section_tables[i].sizeofrawdata);
            g_snprintf (section_name, 6, "%c%c%c%c%c\0", section_tables[i].name, section_tables[i].name >> 8, section_tables[i].name >> 16, section_tables[i].name >> 24, section_tables[i].name >> 32);
            if (g_str_has_prefix ((const gchar *) section_name, ".rsrc"))
                rsrc_table = section_tables[i];
        }
    }
   
/*
    offset = dump_section(section_tables, 2, fis, dis, &error);
*/
    
    g_seekable_seek (G_SEEKABLE (fis), rsrc_table.pointertorawdata, G_SEEK_SET, NULL, &error);
    /* TODO: Check for something */
    if (TRUE) 
    {
        goffset root_rva;
        
        root_rva = g_seekable_tell (G_SEEKABLE (fis));
        resource_directory_table tbt, tbn, tbl;
        dump_directory_table (root_rva, fis, error);

    }
    

    return 0;

}

void
extract_icon (goffset root_rva,
              GInputStream *fis,
              GError *error)
{
    /*
     * MSDN states that Windows NT uses the first GROUP_ICON for exe icon so extract it.
     * RT_GROUP_ICON has id of 0x14.
     */
    resource_directory_table    type, name, language;
    resource_directory_entries  e1, e2, e3;
    gint readcount;
    gint i, j, k;
    goffset orig_offset;
    
    orig_offset = g_seekable_tell (G_SEEKABLE (fis));
    g_seekable_seek (G_SEEKABLE (fis), root_rva, G_SEEK_SET, NULL, &error);
    g_assert (error == NULL);
    
    readcount = g_input_stream_read (fis, &type, sizeof (resource_directory_table), NULL, &error);
    if (readcount > 0 && error == NULL)
    {
        for (i = 0; i < type.numberofidentries + type.numberofnameentries; i++)
        {
            readcount = g_input_stream_read (fis, &e1, sizeof (resource_directory_entries), NULL, &error);
            if (readcount > 0 && error == NULL)
            {
                if (!e1.name.isname && e1.id == 0x14)
                {
                    if (e1.data.isdirectory)
                    {
                        goffset table_start;
                        table_start = g_seekable_tell (G_SEEKABLE (fis));
                        g_seekable_seek (G_SEEKABLE (fis), root_rva + e1.data.offsetofdirectory, G_SEEK_SET, NULL, &error);
                        g_assert (error == NULL);
                        
                        readcount = g_input_stream_read (fis, &name, sizeof (resource_directory_table), NULL, &error);
                        if (readcount > 0 && error == NULL)
                        {
                            for (k = 0; k < name.numberofidentries + name.numberofnameentries; k++)
                            {
                                readcount = g_input_stream_read (fis, &e2, sizeof (resource_directory_entries), NULL, &error);
                                if (readcound > 0 && error == NULL)
                                {
                                    if (e2.data.isdirectory)
                                    {
                                        goffset table2_start;
                                        table2_start = g_seekable_tell (G_SEEKABLE (fis));
                                        g_seekable_seek (G_SEEKABLE (fis), root_rva + e2.data.offsetofdirectory, G_SEEK_SET, NULL, &error);
                                        g_assert (error == NULL);
                                        
                                        readcount = g_input_stream_read (fis, &language, sizeof (resource_directory_table), NULL, &error);
                                        if (readcount > 0 && error == NULL)
                                        {
                                            /* language data entry */
                                        }
                                        
                                    }
                                    else
                                    {
                                        /* not directory */
                                    }
                                }
                                g_seekable_seek (G_SEEKABLE (fis), table2_start, G_SEEK_SET, NULL, &error);
                                g_assert (error == NULL);
                            }
                        }
                        
                        g_seekable_seek (G_SEEKABLE (fis), table_start, G_SEEK_SET, NULL, &error);
                        g_assert (error == NULL);
                    }
                    else
                    {
                        /* not directory */
                    }
                }
                
            }
        }
    }
    
}

void
dump_memory_location (goffset root_rva,
        goffset base_offset,
        GInputStream *fis,
        GError *error)
{
    goffset orig_offset;
    resource_data_entry data_entry;
    
    orig_offset = g_seekable_tell (G_SEEKABLE (fis));
    
    
    g_seekable_seek (G_SEEKABLE (fis), orig_offset, G_SEEK_SET, NULL, &error);
}

void
dump_directory_table (goffset root_rva,
        GInputStream *fis,
        GError *error)
{
    resource_directory_table    type_table, name_table, language_table;
    resource_directory_entries  entry1, entry2, entry3;
    gint readcount;
    gint    i, j, k;
    goffset orig_offset;
    
    orig_offset  = g_seekable_tell (G_SEEKABLE (fis));
    g_seekable_seek (G_SEEKABLE (fis), root_rva, G_SEEK_SET, NULL, &error);
    g_assert (error == NULL);
    readcount = g_input_stream_read (fis, &type_table, sizeof (resource_directory_table), NULL, &error);
    if (readcount > 0 && error == NULL)
    {   
        for (i = 0; i < type_table.numberofidentries + type_table.numberofnameentries; i++)
        {          
            readcount = g_input_stream_read (fis, &entry1, sizeof (resource_directory_entries), NULL, &error);
            if (readcount > 0 && error == NULL)
            {
                if (entry1.data.directory.isdirectory)
                {
                    goffset offset;
                    
                    offset = g_seekable_tell (G_SEEKABLE (fis));
                    g_seekable_seek (G_SEEKABLE (fis), root_rva + entry1.data.directory.offsetofdirectory, G_SEEK_SET, NULL, &error);
                    g_assert (error == NULL);
                    
                    readcount = g_input_stream_read (fis, &name_table, sizeof (resource_directory_table), NULL, &error);
                    if (readcount > 0 && error == NULL)
                    {
                        for (j = 0; j < name_table.numberofidentries + name_table.numberofnameentries; j++)
                        {
                            readcount = g_input_stream_read (fis, &entry2, sizeof (resource_directory_entries), NULL, &error);
                            if (readcount > 0 && error == NULL)
                            {
                                goffset offset1;
                                
                                offset1 = g_seekable_tell (G_SEEKABLE (fis));
                                g_seekable_seek (G_SEEKABLE (fis), root_rva + entry2.data.directory.offsetofdirectory, G_SEEK_SET, NULL, &error);
                                g_assert (error == NULL);
                                
                                readcount = g_input_stream_read (fis, &language_table, sizeof (resource_directory_table), NULL, &error);
                                if (readcount > 0 && error == NULL)
                                {
                                    for (k = 0; k < language_table.numberofidentries + language_table.numberofnameentries; k++)
                                    {   
                                        readcount = g_input_stream_read (fis, &entry3, sizeof (resource_directory_entries), NULL, &error);
                                        if (readcount > 0 && error == NULL)
                                        {
                                            if (entry1.nameoridrva.name.isname)
                                                g_printf ("\"%s\"", 
                                                        get_resource_string(root_rva, entry1.nameoridrva.name.offsetofname, fis));
                                            else
                                                g_printf ("%s",
                                                        get_resource_type(entry1.nameoridrva.id));
                                            g_printf ("\t");
                                            if (entry2.nameoridrva.name.isname)
                                                g_printf ("Name=\"%s\"", name);
                                            else
                                                g_printf ("Name=%x",
                                                        entry2.nameoridrva.id);
                                            g_printf ("\t");
                                            if (entry3.nameoridrva.name.isname)
                                                g_printf ("Language=\"%s\"",
                                                        get_resource_string(root_rva, entry3.nameoridrva.name.offsetofname, fis));
                                            else
                                                g_printf ("Language=0x%x",
                                                        entry3.nameoridrva.id);
                                            g_printf ("\n");
                                        }
                                    }
                                }
                                g_seekable_seek (G_SEEKABLE (fis), offset1, G_SEEK_SET, NULL, &error);
                                
                            }
                        }
                    }
                    g_seekable_seek (G_SEEKABLE (fis), offset, G_SEEK_SET, NULL, &error);
                }
            }
            printf ("\n");
        }
/*
        for (i = 0; i < table.numberofidentries + table.numberofnameentries; i++)
        {
            resource_directory_entry_row *entry_row;
            entry_row = row_count + entry_rows + i;
            if (entry_row->typeisstring)
            {
                g_warning (entry_row->type.string);
            }
        }
*/
    }
    
    g_seekable_seek (G_SEEKABLE (fis), orig_offset, G_SEEK_SET, NULL, &error);
    
}
