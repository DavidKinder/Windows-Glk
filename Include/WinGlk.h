/*
   Header file for Windows specific Glk features.
   Glk API version 0.7.6, WinGlk release 1.56.
*/

#ifndef WINGLK_H_
#define WINGLK_H_

#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WINGLK_BUILD_NUMBER 156

/* Function to be implemented in the Glk program. */
int winglk_startup_code(const char* cmdline);

/* Initialize Windows Glk, or fail with an error message. */
int InitGlk(unsigned int version);

/* Windows Glk specific functions. */
strid_t winglk_stream_open_resource(const char* name, const char* type, glui32 rock);
void winglk_app_set_name(const char* name);
void winglk_window_set_title(const char* title);
void winglk_set_resource_directory(const char* dir);
const char* winglk_get_initial_filename(const char* cmdline, const char* title, const char* filter);
void winglk_set_gui(unsigned int id);
void winglk_load_config_file(const char* gamename);
void* winglk_get_resource_handle(void);
void winglk_set_about_text(const char* text);
void winglk_set_menu_name(const char* name);
void winglk_set_help_file(const char* filename);
frefid_t winglk_fileref_create_by_name(glui32 usage, char *name, glui32 rock, int validate);
void winglk_show_game_dialog(void);

struct winglk_graphic_format
{
  /* The identifier for the format in a Blorb file, e.g. for PNG images,
     this is "giblorb_make_id('P','N','G',' ')". */
  glui32 id;
  /* The string extension for file names when loading graphics not in a
     Blorb file, e.g. for PNG images, this is "png". */
  const char* ext;

  /* This function is called by Windows Glk to create a graphic from its
     resource. "data" gives the byte addresses of the resource and "len"
     its length.
     If successful, the function should set the width and height of the
     graphic to the variables pointed to by the "width" and "height"
     arguments, and return a non-zero "id" value that is passed to the
     "draw" function to identify the graphic.
     If unsuccessful, the function should return zero. */
  DWORD_PTR (*create)(BYTE* data, int len, int* width, int* height);
  /* This function is called by Windows Glk when a graphic has been drawn
     and should be destroyed. */
  void (*destroy)(DWORD_PTR id);
  /* This function is called by Windows Glk to draw a graphic. The graphic
     should be drawn into the buffer pointed to by the "pixels" argument
     in BGRA format: pixels[0] is the blue component of the first pixel;
     pixels[1] the green component; pixels[2] the red component; and
     pixels[3] the alpha channel (or 0xFF if the graphic is completely
     opaque). */
  int (*draw)(DWORD_PTR id, BYTE* pixels);
};

void winglk_add_graphic_format(struct winglk_graphic_format* format);

struct winglk_sound
{
  /* The number of channels, either one (mono) or two (stereo). */
  int channels;
  /* The frequency, in Hz. */
  int freq;
  /* The number of bits per sample, either 8 or 16. */
  int bits;
  /* The duration of the sound, in milliseconds. */
  int duration;
};

struct winglk_sound_format
{
  /* The identifier for the format in a Blorb file, e.g. for Ogg Vorbis
     sound files, this is "giblorb_make_id('O','G','G','V')". */
  glui32 id;
  /* The string prefixing file names when loading sounds not in a Blorb
     file. This is usually "snd" or "mus". */
  const char* prefix;
  /* The string extension for file names when loading sounds not in a
     Blorb file, e.g. for Ogg Vorbis sound files, this is "ogg". */
  const char* ext;

  /* This function is called by Windows Glk to create a sound from its
     resource. "data" gives the byte addresses of the resource and "len"
     its length.
     If successful, the function should set the values in the structure
     pointed to by the "sound" argument, and return a non-zero "id"
     value that is passed to the "write" function to identify the sound
     If unsuccessful, the function should return zero. */
  DWORD_PTR (*create)(BYTE* data, int len, struct winglk_sound* sound);
  /* This function is called by Windows Glk when a sound is no longer
     needed and should be destroyed. */
  void (*destroy)(DWORD_PTR id);
  /* This function is called by Windows Glk when some of the decoded sound
     sample data is to be played. The buffer to fill with sample data, and
     its length in bytes, is given by the "buffer" and "len" arguments.
     If the end of the sound is reached before the buffer is full, the
     value in "*repeat" should be checked. If it is negative, the sound
     repeats indefinitely, and the remainder of the buffer should be filled
     by decoding from the start of the sound. If "*repeat" is zero, the
     remainder of the buffer should be filled with silence (0x80 for bits=8,
     0x00 for bits=16). If "*repeat" is positive it should be decremented by
     one, and the remainder of the buffer should be filled by decoding from
     the start. */
  void (*play)(DWORD_PTR id, unsigned char* buffer, int len, int* repeat);
};

void winglk_add_sound_format(struct winglk_sound_format* format);

/* Windows Glk specific events. */
#define winglk_evtype_GuiInput (0x80000000)

/* Somewhat official Unix extensions. */
const char *glkunix_fileref_get_filename(frefid_t fref);

/* Unofficial Glk extensions. */
void sglk_set_basename(char *s);

/* Gargoyle Glk extensions. */

#define GLK_MODULE_GARGLKTEXT
#define gestalt_GarglkText (0x1100)

void garglk_set_zcolors(glui32 fg, glui32 bg);
void garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg);
void garglk_set_reversevideo(glui32 reverse);
void garglk_set_reversevideo_stream(strid_t str, glui32 reverse);

#define zcolor_Default (-1)
#define zcolor_Current (-2)

#ifdef __cplusplus
}
#endif

#endif /* WINGLK_H_ */
