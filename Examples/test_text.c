#include <stdio.h>
#include <string.h>
#include "glk.h"

#ifndef NULL
#define NULL 0
#endif

#define CMDBUFLEN (64)
#define BUFLEN (256)

static winid_t mainwin = NULL;

typedef void (*testfuncptr_t)(void);

typedef struct testdescriptor_s {
  char *name;
  char *desc;
  testfuncptr_t func;
} testdescriptor_t;

static void print_test_list(void);
static void test_print(void);
static void test_charset(void);
static void test_unicode(void);
static void test_charinput(void);
static void test_unicharinput(void);
static void test_lineinput(void);
static void test_unilineinput(void);
static void test_caseinput(void);
static void test_memstream(void);
static void test_quit(void);

static testdescriptor_t testlist[] = {
  { "print", "Test the basic printing calls", test_print },
  { "charset", "Print the printable Latin-1 characters", test_charset },
  { "unicode", "Print sample strings in many languages", test_unicode },
  { "charinput", "Read single Latin-1 characters", test_charinput },
  { "lineinput", "Read lines of Latin-1 characters", test_lineinput },
  { "unicharinput", "Read single Unicode characters", test_unicharinput },
  { "unilineinput", "Read lines of Unicode characters", test_unilineinput },
  { "caseinput", "Change the case of user-supplied text", test_caseinput },
  { "memstream", "Test memory streams", test_memstream },
  { "quit", "Call glk_exit", test_quit },
  { NULL, NULL, NULL }
};

void glk_main()
{
  char cmdbuf[CMDBUFLEN];
  int ix, jx;

  mainwin = glk_window_open(NULL, 0, 0, wintype_TextBuffer, 1);
  if (!mainwin)
    return;

  glk_set_window(mainwin);

  glk_put_string("Glk API test suite.\n\n");
  print_test_list();
  glk_put_string("\n");

  while (1) {
    int gotline;
    int len;
    event_t ev;
    testfuncptr_t testfunc;

    glk_put_string("> ");
    glk_request_line_event(mainwin, cmdbuf, CMDBUFLEN, 0);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_LineInput:
        gotline = 1;
        break;
      }
    }

    len = ev.val1;

    if (!len) {
      continue;
    }

    if (len == 1) {
      if (cmdbuf[0] == 'q' || cmdbuf[0] == '.') {
        test_quit();
        continue;
      }
    }

    testfunc = NULL;
    for (ix=0; testlist[ix].name; ix++) {
      jx = strlen(testlist[ix].name);
      if (jx == len && !strncmp(testlist[ix].name, cmdbuf, jx)) {
        testfunc = testlist[ix].func;
      }
    }

    if (!testfunc) {
      print_test_list();
      glk_put_string("\n");
      continue;
    }

    (*testfunc)();
    glk_put_string("\n");
  }
}

static void print_test_list()
{
  int ix;

  glk_put_string("Available tests: (\"?\" to repeat this list)\n\n");

  for (ix=0; testlist[ix].name; ix++) {
    glk_put_string(testlist[ix].name);
    glk_put_string(": ");
    glk_put_string(testlist[ix].desc);
    glk_put_string("\n");
  }
}

void print_num(int val)
{
  char buf[16];
  sprintf(buf, "%d", val);
  glk_put_string(buf);
}

void print_hexnum(int val)
{
  char buf[16];
  sprintf(buf, "%X", val);
  glk_put_string(buf);
}

void test_print()
{
  char *str, *cx;
  glui32 longbuf[128];
  glui32 *lx;
  int len;

  glk_put_string("A line of text, printed with glk_put_string.\n");

  str = "A line of text, printed with glk_put_buffer.\n";
  len = strlen(str);
  glk_put_buffer(str, len);

  str = "A line of text, printed char-by-char with glk_put_char.\n";
  for (cx=str; *cx; cx++) {
    glk_put_char(*cx);
  }

#ifdef GLK_MODULE_UNICODE

  str = "Another line of text, printed with glk_put_string_uni.\n";
  for (cx=str, lx=longbuf; *cx; cx++, lx++) {
    *lx = (glui32)(*cx);
  }
  *lx = 0;
  glk_put_string_uni(longbuf);

  str = "A line of text, printed with glk_put_buffer_uni.\n";
  for (cx=str, lx=longbuf; *cx; cx++, lx++) {
    *lx = (glui32)(*cx);
  }
  glk_put_buffer_uni(longbuf, strlen(str));

  str = "A line of text, printed char-by-char with glk_put_char_uni.\n";
  for (cx=str; *cx; cx++) {
    glk_put_char_uni((glui32)(*cx));
  }
  
#else

  glk_put_string("The Unicode printing functions are not available in this library.\n");

#endif
}

void test_charset()
{
  int ix;

  glk_put_string("0-31: (not printable)");

  for (ix=32; ix<127; ix++) {
    if (ix % 16 == 0) {
      glk_put_string("\n");
      if (ix < 100)
        glk_put_string(" ");
      print_num(ix);
      glk_put_string(":  ");
    }
    glk_put_char(ix);
    glk_put_string(" ");
  }

  glk_put_string("\n");
  glk_put_string("127-159: (not printable)");

  for (ix=160; ix<256; ix++) {
    if (ix % 16 == 0) {
      glk_put_string("\n");
      if (ix < 100)
        glk_put_string(" ");
      print_num(ix);
      glk_put_string(":  ");
    }
    glk_put_char(ix);
    glk_put_string(" ");
  }

  glk_put_string("\n");
}

void test_charinput()
{
  glk_put_string("Enter single characters. \".\" or \"q\" will exit.\n");

  while (1) {
    int gotline;
    glui32 ch;
    event_t ev;
    testfuncptr_t testfunc;

    glk_put_string("charinput> ");
    glk_request_char_event(mainwin);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_CharInput:
        gotline = 1;
	ch = ev.val1;
        break;
      }
    }

    if (ch > 0x80000000) {
      glk_put_string("Special keycode\n");
    }
    else {
      glk_put_string("Character: \"");
      glk_put_char(ch);
      glk_put_string("\"\n");
    }

    glk_put_string("Char hex value: ");
    print_hexnum(ch);
    glk_put_string("\n");

    if (ch == '.' || ch == 'q' || ch == 'Q')
      break;
  }
}

void test_unicharinput()
{
#ifdef GLK_MODULE_UNICODE

  glk_put_string("Enter single characters. \".\" or \"q\" will exit.\n");

  while (1) {
    int gotline;
    glui32 ch;
    event_t ev;
    testfuncptr_t testfunc;

    glk_put_string("unicharinput> ");
    glk_request_char_event_uni(mainwin);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_CharInput:
        gotline = 1;
	ch = ev.val1;
        break;
      }
    }

    if (ch > 0x80000000) {
      glk_put_string("Special keycode\n");
    }
    else {
      glk_put_string("Character: \"");
      glk_put_char_uni(ch);
      glk_put_string("\"\n");
    }

    glk_put_string("Char hex value: ");
    print_hexnum(ch);
    glk_put_string("\n");

    if (ch == '.' || ch == 'q' || ch == 'Q')
      break;
  }

#else

  glk_put_string("The Unicode input functions are not available in this library.\n");

#endif /* GLK_MODULE_UNICODE */
}

void test_lineinput()
{
  unsigned char buf[BUFLEN];

  glk_put_string("Enter lines of text. \".\" or \"q\" will exit.\n");

  while (1) {
    int gotline;
    int len, ix;
    glui32 ch;
    event_t ev;

    glk_put_string("lineinput> ");
    glk_request_line_event(mainwin, buf, BUFLEN, 0);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_LineInput:
        gotline = 1;
        break;
      }
    }

    len = ev.val1;

    print_num(len);
    glk_put_string(" characters:\n");
    for (ix=0; ix<len; ix++) {
      ch = buf[ix];
      print_hexnum(ch);
      glk_put_string(" ");      
    }
    glk_put_string("\n");

    glk_put_string("Whole string: \"");
    for (ix=0; ix<len; ix++) {
      ch = buf[ix];
      glk_put_char(ch);
    }
    glk_put_string("\"\n");

    if (len == 1) {
      ch = buf[0];
      if (ch == '.' || ch == 'q' || ch == 'Q')
	break;
    }
  }
}

void test_unilineinput()
{
#ifdef GLK_MODULE_UNICODE

  glui32 buf[BUFLEN];

  glk_put_string("Enter lines of text. \".\" or \"q\" will exit.\n");

  while (1) {
    int gotline;
    int len, ix;
    glui32 ch;
    event_t ev;

    glk_put_string("unilineinput> ");
    glk_request_line_event_uni(mainwin, buf, BUFLEN, 0);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_LineInput:
        gotline = 1;
        break;
      }
    }

    len = ev.val1;

    print_num(len);
    glk_put_string(" characters:\n");
    for (ix=0; ix<len; ix++) {
      ch = buf[ix];
      print_hexnum(ch);
      glk_put_string(" ");      
    }
    glk_put_string("\n");

    glk_put_string("Whole string: \"");
    for (ix=0; ix<len; ix++) {
      ch = buf[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");

    if (len == 1) {
      ch = buf[0];
      if (ch == '.' || ch == 'q' || ch == 'Q')
	break;
    }
  }

#else

  glk_put_string("The Unicode input functions are not available in this library.\n");

#endif /* GLK_MODULE_UNICODE */
}

typedef struct unicodesample_s {
  char *name;
  glui32 *sample;
} unicodesample_t;

/* All these examples are taken from 
   <http://www.unicode.org/standard/WhatIsUnicode.html>. Look at that
   page (in a Unicode-enabled web browser) to see what the strings should
   look like.
*/

static glui32 uni_english_sample[] = { 
  'W', 'h', 'a', 't', ' ', 'i', 's', ' ', 
  'U', 'n', 'i', 'c', 'o', 'd', 'e', '?', 0 
};
static glui32 uni_spanish_sample[] = {
  0xbf, 0x51, 0x75, 0xe9, 0x20, 0x65, 0x73, 0x20,
  0x55, 0x6e, 0x69, 0x63, 0x6f, 0x64, 0x65, 0x3f, 
  0
};
static glui32 uni_croatian_sample[] = {
  0x160, 0x74, 0x6f, 0x20, 0x6a, 0x65, 0x20, 0x55,
  0x6e, 0x69, 0x63, 0x6f, 0x64, 0x65, 0x3f, 0
};
static glui32 uni_russian_sample[] = {
  0x427, 0x442, 0x43e, 0x20, 0x442, 0x430, 0x43a, 0x43e, 
  0x435, 0x20, 0x55, 0x6e, 0x69, 0x63, 0x6f, 0x64, 
  0x65, 0x3f, 0 
};
static glui32 uni_hebrew_sample[] = {
  0x29, 0x3f, 0x55, 0x6e, 0x69, 0x63, 0x6f, 0x64,
  0x65, 0x5de, 0x5d4, 0x20, 0x5d6, 0x5d4, 0x20, 0x5d9,
  0x5d5, 0x5e0, 0x5d9, 0x5e7, 0x5d5, 0x5d3, 0x20, 0x28, 
  0
};
static glui32 uni_japanese_sample[] = {
  0x30e6, 0x30cb, 0x30b3, 0x30fc, 0x30c9, 0x3068, 0x306f, 0x4f55, 
  0x304b, 0xff1f, 0
};
static glui32 uni_korean_sample[] = {
  0xc720, 0xb2c8, 0xcf54, 0xb4dc, 0xc5d0, 0x20, 0xb300, 0xd574, 
  0x3f, 0
};
static glui32 uni_greek_sample[] = {
  0x3a4, 0x3b9, 0x20, 0x3b5, 0x3af, 0x3bd, 0x3b1, 0x3b9,
  0x20, 0x3c4, 0x3bf, 0x20, 0x55, 0x6e, 0x69, 0x63,
  0x6f, 0x64, 0x65, 0x3f, 0
};
static glui32 uni_simpchinese_sample[] = {
  0x4ec0, 0x4e48, 0x662f, 0x55, 0x6e, 0x69, 0x63, 0x6f,
  0x64, 0x65, 0x28, 0x7edf, 0x4e00, 0x7801, 0x29, 0x3f,
  0
};
static glui32 uni_tradchinese_sample[] = {
  0x4ec0, 0x9ebd, 0x662f, 0x55, 0x6e, 0x69, 0x63, 0x6f,
  0x64, 0x65, 0x28, 0x7d71, 0x4e00, 0x78bc, 0x2f, 0x6a19,
  0x6e96, 0x842c, 0x570b, 0x78bc, 0x29, 0x3f, 0
};

void test_unicode()
{
#ifdef GLK_MODULE_UNICODE

  int ix;

  unicodesample_t samples[] = {
    { "English", uni_english_sample },
    { "Spanish", uni_spanish_sample },
    { "Croatian", uni_croatian_sample },
    { "Greek", uni_greek_sample },
    { "Russian", uni_russian_sample },
    { "Hebrew", uni_hebrew_sample },
    { "Japanese", uni_japanese_sample },
    { "Korean", uni_korean_sample },
    { "Simplified Chinese", uni_simpchinese_sample },
    { "Traditional Chinese", uni_tradchinese_sample },
    { NULL, NULL }
  };

  for (ix=0; samples[ix].name; ix++) {
    glk_put_string(samples[ix].name);
    glk_put_string(": ");
    glk_put_string_uni(samples[ix].sample);
    glk_put_string("\n");
  }

#else

  glk_put_string("The Unicode printing functions are not available in this library.\n");

#endif
}

void test_caseinput()
{
#ifdef GLK_MODULE_UNICODE

  glui32 buf[BUFLEN];
  glui32 buf2[BUFLEN];

  glk_put_string("Enter lines of text. \".\" or \"q\" will exit.\n");

  while (1) {
    int gotline;
    int len, ix;
    glui32 ch;
    glui32 res;
    event_t ev;

    glk_put_string("caseinput> ");
    glk_request_line_event_uni(mainwin, buf, BUFLEN, 0);
    gotline = 0;

    while (!gotline) {
      glk_select(&ev);
      switch (ev.type) {
      case evtype_LineInput:
        gotline = 1;
        break;
      }
    }

    len = ev.val1;

    glk_put_string("Orig:  ");
    print_num(len);
    glk_put_string(" characters:");

    glk_put_string(" \"");
    for (ix=0; ix<len; ix++) {
      ch = buf[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");


    memcpy(buf2, buf, sizeof(glui32) * len);
    res = glk_buffer_to_lower_case_uni(buf2, BUFLEN, len);

    glk_put_string("lower: ");
    print_num(res);
    glk_put_string(" characters:");

    glk_put_string(" \"");
    for (ix=0; ix<res; ix++) {
      ch = buf2[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");


    memcpy(buf2, buf, sizeof(glui32) * len);
    res = glk_buffer_to_upper_case_uni(buf2, BUFLEN, len);

    glk_put_string("UPPER: ");
    print_num(res);
    glk_put_string(" characters:");

    glk_put_string(" \"");
    for (ix=0; ix<res; ix++) {
      ch = buf2[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");


    memcpy(buf2, buf, sizeof(glui32) * len);
    res = glk_buffer_to_title_case_uni(buf2, BUFLEN, len, 0);

    glk_put_string("T....: ");
    print_num(res);
    glk_put_string(" characters:");

    glk_put_string(" \"");
    for (ix=0; ix<res; ix++) {
      ch = buf2[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");


    memcpy(buf2, buf, sizeof(glui32) * len);
    res = glk_buffer_to_title_case_uni(buf2, BUFLEN, len, 1);

    glk_put_string("Title: ");
    print_num(res);
    glk_put_string(" characters:");

    glk_put_string(" \"");
    for (ix=0; ix<res; ix++) {
      ch = buf2[ix];
      glk_put_char_uni(ch);
    }
    glk_put_string("\"\n");


    if (len == 1) {
      ch = buf[0];
      if (ch == '.' || ch == 'q' || ch == 'Q')
	break;
    }
  }

#else

  glk_put_string("The Unicode printing functions are not available in this library.\n");

#endif
}

static int expect_stream_result(stream_result_t *res, glui32 rd, glui32 wr)
{
  int errcount = 0;

  if ((res)->readcount != rd) {
    errcount++;
    glk_put_string("Read count ");
    print_num((res)->readcount);
    glk_put_string(" instead of ");
    print_num(rd);
    glk_put_string("\n");
  }

  if ((res)->writecount != wr) {
    errcount++;
    glk_put_string("Write count ");
    print_num((res)->writecount);
    glk_put_string(" instead of ");
    print_num(wr);
    glk_put_string("\n");
  }

  return errcount;
}

void test_memstream()
{
  char charbuf1[] = "Hello.\nGoodbye."; /* 15 chars */
  char charbuf2[] = "\307lich\351, \245en for \243\370\339\n"; /* 20 chars */
  char charbuf[64];
  glui32 ucharbuf1[] = { 'H', 'e', 'l', 'l', 'o', '.', '\n',
    'G', 'o', 'o', 'd', 'b', 'y', 'e', '.' }; /* 15 chars */
  glui32 ucharbuf2[] = { 
    0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, ' ',
    0x3BB, 0x3BF, 0x3B3, 0x3B9, 0x3C3, 0x3BC, 0x3B9, 0x3BA, 0x3CC, ' ',
    0x5EA, 0x5D5, 0x5DB, 0x5E0, 0x5D9, 0x5EA, ' ',
    0x7A0B, 0x5F0F, ' ',
    0x43F, 0x440, 0x43E, 0x433, 0x440, 0x430, 0x43C, 0x43C, 0x44B, '\n'
  }; /* 38 chars */
  char ucharbuf2flat[] = "program ????????? ?????? ?? ?????????\n";
  glui32 ucharbufhello[] = { 'H', 'e', 'l', 'l', 'o', '.', 'Q' };
  glui32 ucharbufgoodbye[] = { 'G', 0xF6, 'o', 'd', 'b', 'y', 'e', '.', 0 };
  glui32 ucharbufxhellogoodbye[] = { 'X', 
    'H', 'e', 'l', 'l', 'o', '.',
    'G', 0xF6, 'o', 'd', 'b', 'y', 'e', '.' }; /* 15 chars */
  glui32 ucharbufhebrew[] = {
    0x5EA, 0x5D5, 0x5DB, 0x5E0, 0x5D9, 0x5EA 
  };
  glui32 ucharbufgreek[] = {
    0x3BB, 0x3BF, 0x3B3, 0x3B9, 0x3C3, 0x3BC, 0x3B9, 0x3BA, 0x3CC, 0
  };
  glui32 ucharbuffancy[] = {
    0x7A0B, 0x5F0F,
    0x5EA, 0x5D5, 0x5DB, 0x5E0, 0x5D9, 0x5EA,
    0x3BB, 0x3BF, 0x3B3, 0x3B9, 0x3C3, 0x3BC, 0x3B9, 0x3BA, 0x3CC
  };

  glui32 ucharbuf[64];
  strid_t str;
  stream_result_t result;
  glui32 val, lx;
  char *cx;
  int errcount = 0;

  glk_put_string("Performing silent tests...\n");


  glk_put_string("...Unread stream\n");

  str = glk_stream_open_memory(charbuf1, 15, filemode_Read, 0);
  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 0);


  glk_put_string("...Get_buffer\n");

  str = glk_stream_open_memory(charbuf1, 15, filemode_Read, 0);

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 15) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 15.\n");
  }
  if (memcmp(charbuf, charbuf1, 15)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 0) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 0.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_buffer twice\n");

  str = glk_stream_open_memory(charbuf1, 15, filemode_Read, 0);

  val = glk_get_buffer_stream(str, charbuf, 6);
  if (val != 6) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 6.\n");
  }
  if (memcmp(charbuf, charbuf1, 6)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 9) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 9.\n");
  }
  if (memcmp(charbuf, charbuf1+6, 9)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_line\n");

  str = glk_stream_open_memory(charbuf1, 15, filemode_Read, 0);

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 7) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 7.\n");
  }
  if (memcmp(charbuf, charbuf1, 7)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 8) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 8.\n");
  }
  if (memcmp(charbuf, charbuf1+7, 8)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_char\n");

  str = glk_stream_open_memory(charbuf1, 15, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream(str);
    if (ch == -1)
      break;
    if (ch != charbuf1[lx]) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 15) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 15.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_buffer with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 20) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 20.\n");
  }
  if (memcmp(charbuf, charbuf2, 20)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


  glk_put_string("...Get_line with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 20) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 20.\n");
  }
  if (memcmp(charbuf, charbuf2, 20)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


  glk_put_string("...Get_char with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream(str);
    if (ch == -1)
      break;
    if (ch != (unsigned char)(charbuf2[lx])) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 20) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 20.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


#ifdef GLK_MODULE_UNICODE


  glk_put_string("...Get_buffer_uni with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  val = glk_get_buffer_stream_uni(str, ucharbuf, 64);
  if (val != 20) {
    errcount++;
    glk_put_string("get_buffer_stream_uni value was ");
    print_num(val);
    glk_put_string(" instead of 20.\n");
  }
  for (lx=0; lx<20; lx++) {
    if (ucharbuf[lx] != (unsigned char)(charbuf2[lx])) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


  glk_put_string("...Get_line_uni with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  val = glk_get_line_stream_uni(str, ucharbuf, 64);
  if (val != 20) {
    errcount++;
    glk_put_string("get_line_stream_uni value was ");
    print_num(val);
    glk_put_string(" instead of 20.\n");
  }
  for (lx=0; lx<20; lx++) {
    if (ucharbuf[lx] != (unsigned char)(charbuf2[lx])) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


  glk_put_string("...Get_char_uni with high-bit char\n");

  str = glk_stream_open_memory(charbuf2, 20, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream_uni(str);
    if (ch == -1)
      break;
    if (ch != (unsigned char)(charbuf2[lx])) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 20) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 20.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 20, 0);


  glk_put_string("...Get_buffer_uni with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  val = glk_get_buffer_stream_uni(str, ucharbuf, 64);
  if (val != 15) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 15.\n");
  }
  if (memcmp(ucharbuf, ucharbuf1, 15 * sizeof(glui32))) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_line_uni with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  val = glk_get_line_stream_uni(str, ucharbuf, 64);
  if (val != 7) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 7.\n");
  }
  if (memcmp(ucharbuf, ucharbuf1, 7 * sizeof(glui32))) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  val = glk_get_line_stream_uni(str, ucharbuf, 64);
  if (val != 8) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 8.\n");
  }
  if (memcmp(ucharbuf, ucharbuf1+7, 8 * sizeof(glui32))) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_char_uni with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream_uni(str);
    if (ch == -1)
      break;
    if (ch != ucharbuf1[lx]) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 15) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 15.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_buffer_uni with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  val = glk_get_buffer_stream_uni(str, ucharbuf, 64);
  if (val != 38) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 38.\n");
  }
  if (memcmp(ucharbuf, ucharbuf2, 38 * sizeof(glui32))) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


  glk_put_string("...Get_line_uni with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  val = glk_get_line_stream_uni(str, ucharbuf, 64);
  if (val != 38) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 38.\n");
  }
  if (memcmp(ucharbuf, ucharbuf2, 38 * sizeof(glui32))) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


  glk_put_string("...Get_char_uni with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream_uni(str);
    if (ch == -1)
      break;
    if (ch != ucharbuf2[lx]) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 38) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 38.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


  glk_put_string("...Get_buffer with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 15) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 15.\n");
  }
  for (lx=0; lx<15; lx++) {
    if ((unsigned char)(charbuf[lx]) != ucharbuf1[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_line with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 7) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 7.\n");
  }
  for (lx=0; lx<7; lx++) {
    if ((unsigned char)(charbuf[lx]) != ucharbuf1[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 8) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 8.\n");
  }
  for (lx=0; lx<8; lx++) {
    if ((unsigned char)(charbuf[lx]) != ucharbuf1[7+lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_char with unicode stream\n");

  str = glk_stream_open_memory_uni(ucharbuf1, 15, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream(str);
    if (ch == -1)
      break;
    if (ch != ucharbuf1[lx]) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 15) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 15.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 15, 0);


  glk_put_string("...Get_buffer with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  val = glk_get_buffer_stream(str, charbuf, 64);
  if (val != 38) {
    errcount++;
    glk_put_string("get_buffer_stream value was ");
    print_num(val);
    glk_put_string(" instead of 38.\n");
  }
  for (lx=0; lx<38; lx++) {
    if ((unsigned char)(charbuf[lx]) != ucharbuf2flat[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


  glk_put_string("...Get_line with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  val = glk_get_line_stream(str, charbuf, 64);
  if (val != 38) {
    errcount++;
    glk_put_string("get_line_stream value was ");
    print_num(val);
    glk_put_string(" instead of 38.\n");
  }
  for (lx=0; lx<38; lx++) {
    if ((unsigned char)(charbuf[lx]) != ucharbuf2flat[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


  glk_put_string("...Get_char with unicode stream with fancy chars\n");

  str = glk_stream_open_memory_uni(ucharbuf2, 38, filemode_Read, 0);

  for (lx = 0; 1; lx++) {
    glsi32 ch;
    ch = glk_get_char_stream(str);
    if (ch == -1)
      break;
    if (ch != ucharbuf2flat[lx]) {
      errcount++;
      glk_put_string("Chars do not match.\n");
    }
  }
  if (lx != 38) {
    errcount++;
    glk_put_string("get_char count was ");
    print_num(lx);
    glk_put_string(" instead of 38.\n");
  }

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 38, 0);


#endif /* GLK_MODULE_UNICODE */


  glk_put_string("...Put to memory stream\n");

  str = glk_stream_open_memory(charbuf, 64, filemode_Write, 0);
  
  glk_put_char_stream(str, 'X');
  glk_put_buffer_stream(str, "Hello.Q", 6);
  glk_put_string_stream(str, "G\366\366dbye.");

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 15);

  if (memcmp(charbuf, "XHello.G\366\366dbye.", 15)) {
    glk_put_string("Strings do not match.\n");
    errcount++;
  }


#ifdef GLK_MODULE_UNICODE


  glk_put_string("...Put to unicode memory stream\n");

  str = glk_stream_open_memory_uni(ucharbuf, 64, filemode_Write, 0);
  
  glk_put_char_stream(str, 'X');
  glk_put_buffer_stream(str, "Hello.Q", 6);
  glk_put_string_stream(str, "G\366\366dbye.");

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 15);

  cx = "XHello.G\366\366dbye.";
  for (lx=0; lx<15; lx++) {
    if ((unsigned char)(cx[lx]) != ucharbuf[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }


  glk_put_string("...Put_uni to unicode memory stream\n");

  str = glk_stream_open_memory_uni(ucharbuf, 64, filemode_Write, 0);
  
  glk_put_char_stream_uni(str, 'X');
  glk_put_buffer_stream_uni(str, ucharbufhello, 6);
  glk_put_string_stream_uni(str, ucharbufgoodbye);

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 15);

  for (lx=0; lx<15; lx++) {
    if (ucharbufxhellogoodbye[lx] != ucharbuf[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }


  glk_put_string("...Put_uni fancy to unicode memory stream\n");

  str = glk_stream_open_memory_uni(ucharbuf, 64, filemode_Write, 0);
  
  glk_put_char_stream_uni(str, 0x7A0B);
  glk_put_char_stream_uni(str, 0x5F0F);
  glk_put_buffer_stream_uni(str, ucharbufhebrew, 6);
  glk_put_string_stream_uni(str, ucharbufgreek);

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 17);

  for (lx=0; lx<17; lx++) {
    if (ucharbuffancy[lx] != ucharbuf[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }


  glk_put_string("...Put_uni fancy to memory stream\n");

  str = glk_stream_open_memory(charbuf, 64, filemode_Write, 0);
  
  glk_put_char_stream_uni(str, 0x7A0B);
  glk_put_char_stream_uni(str, 0x5F0F);
  glk_put_buffer_stream_uni(str, ucharbufhebrew, 6);
  glk_put_string_stream_uni(str, ucharbufgreek);

  glk_stream_close(str, &result);
  errcount += expect_stream_result(&result, 0, 17);

  cx = "?????????????????";
  for (lx=0; lx<17; lx++) {
    if (cx[lx] != charbuf[lx]) {
      glk_put_string("Chars do not match.\n");
      errcount++;
    }
  }


#endif /* GLK_MODULE_UNICODE */


  if (errcount == 0) {
    glk_put_string("Tests succeeded.\n");
  }
  else {
    glk_put_string("Failed with ");
    print_num(errcount);
    glk_put_string(" errors.\n");
  }
}

void test_quit()
{
  glk_put_string("Goodbye.\n");
  glk_exit();
}
