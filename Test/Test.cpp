/* Test harness for Windows Glk */

#define _CRT_SECURE_NO_WARNINGS

extern "C" {
#include "glk.h"
#include "gi_blorb.h"
#include "WinGlk.h"
}

#include <stdio.h>
#include <windows.h>
#include "resource.h"

winid_t main = 0;
winid_t grid = 0;
winid_t title = 0;
schanid_t snd[2] = { 0,0 };

struct TestOutputChar
{
  glui32 theChar;
  glui32 out[2];
  glui32 len[2];
  TestOutputChar(glui32 c) : theChar(c) {}
};
TestOutputChar TestOutputChars[] = { 10,13,65,163,0x3B2,0x404,0x434,0x2654,0x2801,0x20000,keycode_Return,0 };

int winglk_startup_code(const char* cmdline)
{
  winglk_app_set_name("Glk Test Harness");
  return 1;
}

void print_list(void)
{
  glk_put_string("\n");
  glk_put_string(" 0. Show this list\n");
  glk_put_string(" 1. Hyperlinks\n");
  glk_put_string(" 2. Play Sounds\n");
  glk_put_string(" 3. Stop Sounds\n");
  glk_put_string(" 4. Set Sound Volume\n");
  glk_put_string(" 5. Text in Style User 1\n");
  glk_put_string(" 6. Get Hints for Style User 1\n");
  glk_put_string(" 7. Set Base Filename\n");
  glk_put_string(" 8. Change Blorb Resource File\n");
  glk_put_string(" 9. Timer Speed\n");
  glk_put_string("10. Character Output\n");
  glk_put_string("11. Get Window Dimensions\n");
  glk_put_string("12. Unicode Files\n");
  glk_put_string("13. Print a Long Paragraph\n");
  glk_put_string("14. Line input with margin images\n");
  glk_put_string("15. Play Multiple Sounds\n");
  glk_put_string("16. Pause a Sound\n");
  glk_put_string("17. Unpause a Sound\n");
}

void line_input_margins()
{
  glk_image_draw_scaled(main,0,imagealign_MarginLeft,0,128,128);
  glk_image_draw_scaled(main,0,imagealign_MarginRight,0,64,64);
  glk_put_string(">");
  while (true)
  {
    char buffer[512];
    glk_request_line_event(main,buffer,511,0);

    event_t ev;
    glk_select(&ev);
    if ((ev.type == evtype_LineInput) && (ev.win == main))
      return;
  }
}

void print_long_paragraph()
{
  for (int i = 0; i < 400; i++)
  {
    char line[100];
    sprintf(line,"%d) This is a line of text ... ",i);
    glk_put_string(line);
  }
  glk_put_char('\n');
}

#ifdef GLK_MODULE_UNICODE
void test_file_unicode()
{
  static glui32 unicode_data[] = {
    0xbf, 0x51, 0x75, 0xe9, 0x20, 0x65, 0x73, 0x20,
    0x55, 0x6e, 0x69, 0x63, 0x6f, 0x64, 0x65, 0x3f,
    0x427, 0x442, 0x43e, 0x20, 0x442, 0x430, 0x43a, 0x43e, 
    0x435, 0x20, 0x55, 0x6e, 0x69, 0x63, 0x6f, 0x64, 
    0x65, 0x3f, 0 };

  for (int i = 0; i < 2; i++)
  {
    frefid_t fileref;

    if (i == 0)
    {
      glk_put_string("Writing out text Unicode data...\n");
      fileref = glk_fileref_create_by_name(fileusage_TextMode,"TestUnicode.txt",0);
    }
    else
    {
      glk_put_string("Writing out binary Unicode data...\n");
      fileref = glk_fileref_create_by_name(fileusage_BinaryMode,"TestUnicode.bin",0);
    }

    strid_t stream = glk_stream_open_file_uni(fileref,filemode_Write,0);
    glk_put_string_stream_uni(stream,unicode_data);
    glk_stream_close(stream,NULL);

    if (i == 0)
      glk_put_string("Reading back text Unicode data...\n");
    else
      glk_put_string("Reading back binary Unicode data...\n");

    stream = glk_stream_open_file_uni(fileref,filemode_Read,0);
    glui32 buffer[64];
    glui32 read = glk_get_buffer_stream_uni(stream,buffer,64);

    int mismatch = 0;
    for (int i = 0; i < (int)read; i++)
    {
      if (unicode_data[i] != buffer[i])
        mismatch++;
    }
    char msg[256];
    sprintf(msg,"Read %d characters, %d do not match.\n",read,mismatch);
    glk_put_string(msg);

    glk_stream_close(stream,NULL);
    glk_fileref_destroy(fileref);
  }
}
#else
void test_file_unicode()
{
  glk_put_string("Unicode not supported!\n");
}
#endif

void update_character_output(int x)
{
  for (int i = 0; TestOutputChars[i].theChar != 0; i++)
  {
    TestOutputChar& toc = TestOutputChars[i];
    toc.out[x] = glk_gestalt_ext(gestalt_CharOutput,toc.theChar,&(toc.len[x]),1);
  }
}

const char* get_output_str(glui32 out)
{
  switch (out)
  {
  case gestalt_CharOutput_ExactPrint:
    return "exact";
  case gestalt_CharOutput_ApproxPrint:
    return "approx";
  case gestalt_CharOutput_CannotPrint:
    return "cannot";
  }
  return "???";
}

void test_character_output()
{
  update_character_output(1);

  for (int i = 0; TestOutputChars[i].theChar != 0; i++)
  {
    char buffer[256];
    sprintf(buffer,"Character 0x%.8X (%s,%d) (%s,%d): ",(int)TestOutputChars[i].theChar,
      get_output_str(TestOutputChars[i].out[0]),(int)TestOutputChars[i].len[0],
      get_output_str(TestOutputChars[i].out[1]),(int)TestOutputChars[i].len[1]);
    glk_put_string(buffer);

#ifdef GLK_MODULE_UNICODE
    glk_put_char_uni(TestOutputChars[i].theChar);
    glk_put_char_uni(L'\n');
#else
    glk_put_char((char)(TestOutputChars[i].theChar & 255));
    glk_put_char('\n');
#endif
  }
}

void get_window_dimensions()
{
  if (title != 0)
    glk_window_close(title,0);
  title = 0;
  if (grid != 0)
    glk_window_close(grid,0);
  grid = 0;

  glk_window_close(main,0);
  main = glk_window_open(0,0,0,wintype_Graphics,0);

  glui32 x,y;
  glk_window_get_size(main,&x,&y);

  glk_window_close(main,0);
  main = glk_window_open(0,0,0,wintype_TextBuffer,0);
  glk_set_window(main);

  char buffer[256];
  sprintf(buffer,"\nThe size of the window is (%d,%d) pixels, and this text should be on the second line.\n",x,y);
  glk_put_string(buffer);
}

void test_timer()
{
  glk_request_timer_events(1);
  DWORD StartTick = ::GetTickCount();

  event_t ev;
  for (int i = 0; i < 100; i++)
  {
    do
    {
      glk_select(&ev);
    }
    while (ev.type != evtype_Timer);
  }

  DWORD EndTick = ::GetTickCount();
  glk_request_timer_events(0);

  char buffer[256];
  sprintf(buffer,"\nTiming loop took %dms.\n",EndTick-StartTick);
  glk_put_string(buffer);
}

void test_hyperlinks()
{
  glk_stylehint_set(wintype_TextBuffer,style_User2,stylehint_Justification, 
    stylehint_just_Centered);

  if (title != 0)
    glk_window_close(title,0);
  title = glk_window_open(main,winmethod_Above|winmethod_Fixed,1,
    wintype_TextBuffer,0);

  glk_set_window(title);
  glk_set_style(style_User2);
  glk_put_string("Hyperlink Test");

  if (grid != 0)
    glk_window_close(grid,0);
  grid = glk_window_open(main,winmethod_Above|winmethod_Fixed,2,
    wintype_TextGrid,0);

  glk_set_window(grid);
  glk_put_string("  Test of");
  glk_set_style(style_Emphasized);
  glk_put_string(" grid ");
  glk_set_hyperlink(1);
  glk_put_string("hyper");
  glk_set_style(style_Normal);
  glk_put_string("linking");
  glk_set_hyperlink(0);
  glk_put_string("\n More test text ... More test text ... More test text ...");
  glk_set_window(main);
  
  glk_set_style(style_Emphasized);
  glk_put_string("\nHyperlinks\n");
  glk_set_style(style_Normal);

  glk_set_hyperlink(1);
  glk_image_draw(main,0,imagealign_MarginRight,0);
  glk_put_string("Hyperlink 1\n");
  glk_set_hyperlink(0);

  glk_put_string("Testing ");
  glk_set_hyperlink(2);
  glk_put_string("Hyperlink 2");
  glk_set_hyperlink(0);
  glk_put_string(" Testing\n");

  glk_put_string("Testing ");
  glk_set_hyperlink(3);
  glk_put_string("Hyper");
  glk_set_style(style_Emphasized);
  glk_put_string("link 3");
  glk_image_draw_scaled(main,0,imagealign_InlineCenter,0,64,64);
  glk_set_hyperlink(0);
  glk_put_string(" Test");
  glk_set_style(style_Normal);
  glk_put_string("ing\n");

  glk_put_string("Testing ");
  glk_set_hyperlink(4);
  glk_put_string("Hyper\nlink 4");
  glk_set_hyperlink(5);
  glk_put_string("Hyperlink 5");
  glk_set_hyperlink(0);
  glk_put_string(" Testing\n");
}

int get_number(char* prompt)
{
  glk_put_string(prompt);
  while (true)
  {
    char buffer[256];
    glk_request_line_event(main,buffer,255,0);

    event_t ev;
    glk_select(&ev);
    if (ev.type == evtype_LineInput)
    {
      if (ev.win == main)
      {
        buffer[ev.val1] = 0;

        int number = 0;
        if (sscanf(buffer,"%d",&number) == 1)
          return number;
      }
    }
  }
  return 0;
}

void test_sound(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nSound Playing\n");
  glk_set_style(style_Normal);

  int channel = get_number("Enter channel (0 or 1): ");
  if ((channel < 0) || (channel > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }
  int resource = get_number("Enter sound resource number: ");
  int repeats = get_number("Enter repeats (negative for infinite): ");
  int notify = get_number("Enter notification id (or 0 for none): ");

  int play = glk_schannel_play_ext(snd[channel],resource,repeats,notify);
  if (play == 0)
    glk_put_string("Failed to play sound\n");
  else
    glk_put_string("Sound playing\n");
}

void test_sound_multi(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nMultiple Sound Playing\n");
  glk_set_style(style_Normal);

  glui32 resources[2];
  int channel1 = get_number("Enter first channel (0 or 1): ");
  if ((channel1 < 0) || (channel1 > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }
  resources[0] = get_number("Enter second sound resource number: ");

  int channel2 = get_number("Enter second channel (0 or 1): ");
  if ((channel2 < 0) || (channel2 > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }
  resources[1] = get_number("Enter second sound resource number: ");

  int notify = get_number("Enter notification id (or 0 for none): ");

  schanid_t channels[2];
  channels[0] = snd[channel1];
  channels[1] = snd[channel2];
  int play = glk_schannel_play_multi(channels,2,resources,2,notify);
  if (play == 0)
    glk_put_string("Failed to play sounds\n");
  else
  {
    char buffer[256];
    sprintf(buffer,"%d sounds playing",play);
    glk_put_string(buffer);
  }
}

void test_sound_stop(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nSound Stoping\n");
  glk_set_style(style_Normal);

  for (int i = 0; i < 2; i++)
    glk_schannel_stop(snd[i]);
}

void test_sound_volume(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nSetting Sound Volume\n");
  glk_set_style(style_Normal);

  int channel = get_number("Enter channel (0 or 1): ");
  if ((channel < 0) || (channel > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }

  int volume = get_number("Enter volume (0 to 65536): ");
  if ((volume < 0) || (volume > 65536))
  {
    glk_put_string("Invalid volume\n");
    return;
  }

  int millis = get_number("Enter duration (in milliseconds): ");
  if (millis < 0)
  {
    glk_put_string("Invalid duration\n");
    return;
  }

  int notify = get_number("Enter notification id (or 0 for none): ");

  glk_schannel_set_volume_ext(snd[channel],volume,millis,notify);
}

void test_sound_pause(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nPause a Sound\n");
  glk_set_style(style_Normal);

  int channel = get_number("Enter channel to pause (0 or 1): ");
  if ((channel < 0) || (channel > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }
  glk_schannel_pause(snd[channel]);
}

void test_sound_unpause(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nUnpause a Sound\n");
  glk_set_style(style_Normal);

  int channel = get_number("Enter channel to unpause (0 or 1): ");
  if ((channel < 0) || (channel > 1))
  {
    glk_put_string("Invalid channel\n");
    return;
  }
  glk_schannel_unpause(snd[channel]);
}

void test_user_1(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nText in Style User 1\n");
  glk_set_style(style_Normal);

  glk_put_string("\nStarting test... ");
  for (int i = 0; i < 5; i++)
  {
    glk_set_style(style_User1);
    glk_put_string("This is a test of text being output in user style 1, ");
    glk_set_style(style_Normal);
    glk_put_string("and this is normal style. ");
  }

  glk_put_string("\n\n");
  glk_set_style(style_User1);
  glk_put_string("Enter anything:");

  char buffer[256];
  glk_request_line_event(main,buffer,255,0);
  while (true)
  {
    event_t ev;
    glk_select(&ev);
    if (ev.type == evtype_LineInput)
    {
      buffer[ev.val1] = 0;
      glk_put_string("You entered \"");
      glk_put_string(buffer);
      glk_put_string("\"");
      glk_set_style(style_Normal);
      glk_put_string("\n");
      return;
    }
  }
}

void print_stylehint(const char* text, int hint)
{
  glui32 result;
  char buffer[256];
  if (glk_style_measure(main,style_User1,hint,&result) == 1)
  {
    sprintf(buffer,text,result);
    glk_put_string(buffer);
  }
}

void test_hints_user_1(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nGet Hints for Style User 1\n");
  glk_set_style(style_Normal);

  print_stylehint("Indentation: %d\n",stylehint_Indentation);
  print_stylehint("Paragraph Indentation: %d\n",stylehint_ParaIndentation);
  print_stylehint("Justification: %d\n",stylehint_Justification);
  print_stylehint("Size: %d\n",stylehint_Size);
  print_stylehint("Weight: %d\n",stylehint_Weight);
  print_stylehint("Oblique: %d\n",stylehint_Oblique);
  print_stylehint("Proportional: %d\n",stylehint_Proportional);
  print_stylehint("Text Colour: 0x%.8X\n",stylehint_TextColor);
  print_stylehint("Background Colour: 0x%.8X\n",stylehint_BackColor);
  print_stylehint("Reverse Colour: %d\n",stylehint_ReverseColor);
}

void test_basename(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nSet Base Filename\n");
  glk_set_style(style_Normal);

  glk_put_string("\nNew basename:");
  while (true)
  {
    char buffer[256];
    glk_request_line_event(main,buffer,255,0);

    event_t ev;
    glk_select(&ev);
    if (ev.type == evtype_LineInput)
    {
      if (ev.win == main)
      {
        buffer[ev.val1] = 0;
        sglk_set_basename(buffer);

        frefid_t test = glk_fileref_create_by_prompt(fileusage_Data,filemode_Read,0);
        if (test != 0)
          glk_fileref_destroy(test);

        return;
      }
    }
  }
}

void test_change_blorb(void)
{
  glk_set_style(style_Emphasized);
  glk_put_string("\nSet Blorb Resource File\n");
  glk_set_style(style_Normal);

  frefid_t resref = glk_fileref_create_by_prompt(fileusage_Data,filemode_Read,0);
  if (resref != 0)
  {
    strid_t resfile = glk_stream_open_file(resref,filemode_Read,0);
    if (resfile)
      giblorb_set_resource_map(resfile);
    glk_fileref_destroy(resref);
  }
}

void test(int number)
{
  switch (number)
  {
  case 0:
    print_list();
    break;
  case 1:
    test_hyperlinks();
    break;
  case 2:
    test_sound();
    break;
  case 3:
    test_sound_stop();
    break;
  case 4:
    test_sound_volume();
    break;
  case 5:
    test_user_1();
    break;
  case 6:
    test_hints_user_1();
    break;
  case 7:
    test_basename();
    break;
  case 8:
    test_change_blorb();
    break;
  case 9:
    test_timer();
    break;
  case 10:
    test_character_output();
    break;
  case 11:
    get_window_dimensions();
    break;
  case 12:
    test_file_unicode();
    break;
  case 13:
    print_long_paragraph();
    break;
  case 14:
    line_input_margins();
    break;
  case 15:
    test_sound_multi();
    break;
  case 16:
    test_sound_pause();
    break;
  case 17:
    test_sound_unpause();
    break;
  }
}

void load_res(char* resname)
{
  frefid_t resref = winglk_fileref_create_by_name(fileusage_BinaryMode|fileusage_Data,resname,0,0);
  if (resref)
  {
    strid_t resfile = glk_stream_open_file(resref,filemode_Read,0);
    if (resfile)
      giblorb_set_resource_map(resfile);
    glk_fileref_destroy(resref);
  }
}

void glk_main(void)
{
  update_character_output(0);

  load_res("\\programs\\adv\\glk\\test\\test resources\\test.blb");
  winglk_set_resource_directory("\\programs\\adv\\glk\\test\\test resources");
  winglk_load_config_file("\\programs\\adv\\glk\\test\\test.cfg");
  winglk_set_gui(IDR_TEST);

  main = glk_window_open(0,0,0,wintype_TextBuffer,0);

  snd[0] = glk_schannel_create(0);
  snd[1] = glk_schannel_create_ext(0,0x8000);

  glk_set_window(main);

  glk_set_style(style_Header);
  glk_put_string("WinGlk Test Harness\n");
  glk_set_style(style_Normal);
  print_list();

  bool show = true;
  while (true)
  {
    if (show)
    {
      glk_set_style(style_Normal);
      glk_put_string("\nTest:");
      show = false;
    }

    char buffer[256];
    glk_request_line_event(main,buffer,255,0);
    glk_request_hyperlink_event(main);
    if (grid != 0)
      glk_request_hyperlink_event(grid);

    event_t ev;
    glk_select(&ev);
    switch (ev.type)
    {
    case evtype_LineInput:
      if (ev.win == main)
      {
        buffer[ev.val1] = 0;

        int test_num = 0;
        if (sscanf(buffer,"%d",&test_num) == 1)
          test(test_num);

        show = true;
      }
      break;
    case evtype_Hyperlink:
      if (ev.win == main)
      {
        glk_cancel_line_event(main,NULL);
        sprintf(buffer,"Hyperlink %d selected",ev.val1);
        glk_put_string(buffer);
        show = true;
      }
      else if (ev.win == grid)
      {
        glk_window_close(grid,0);
        grid = 0;
        glk_window_close(title,0);
        title = 0;

        glk_cancel_line_event(main,NULL);
        sprintf(buffer,"Grid hyperlink %d selected",ev.val1);
        glk_put_string(buffer);
        show = true;
      }
      break;
    case evtype_SoundNotify:
      glk_cancel_line_event(main,NULL);
      sprintf(buffer,"Sound %d finished playing, notification id %d",ev.val1,ev.val2);
      glk_put_string(buffer);
      show = true;
      break;
    case evtype_VolumeNotify:
      glk_cancel_line_event(main,NULL);
      sprintf(buffer,"Volume change finished, notification id %d",ev.val2);
      glk_put_string(buffer);
      show = true;
      break;
    case winglk_evtype_GuiInput:
      glk_cancel_line_event(main,NULL);
      sprintf(buffer,"Menu or toolbar item selected, identifier is %d",ev.val1);
      glk_put_string(buffer);
      show = true;
      break;
    }
  }
}
