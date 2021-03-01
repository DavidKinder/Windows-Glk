# Windows Glk

This is a Windows implementation of Andrew Plotkin's [Glk](https://eblong.com/zarf/glk/index.html) library specification, an attempt to define a portable API for text adventures, and more generally, any program with primarily textual input and output.

In order to produce a useable program, the Glk library is combined with some sort of text adventure player (such as [Glulxe](https://github.com/erkyrath/glulxe) or [Git](https://github.com/DavidKinder/Git)):

![Windows Glulxe playing Counterfeit Monkey](Counterfeit%20Monkey.png)

## Building

Download and install Visual Studio 2019 Community edition from https://visualstudio.microsoft.com/. In the installer, under "Workloads", make sure that "Desktop development with C++" is selected, and under "Individual components" that "C++ MFC for latest build tools (x86 & x64)" is selected.

Install git. I use the version of git that is part of Cygwin, a Linux-like environment for Windows, but Git for Windows can be used from a Windows command prompt.

Open the environment that you are using git from (e.g. Cygwin), and switch to the root directory that the build environment will be created under (from here referred to as "\<root>"). Clone this and the other required repositories of mine with git:
```
git clone https://github.com/DavidKinder/Windows-Glk.git Adv/Glk
git clone https://github.com/DavidKinder/Libraries.git Libraries
```

### Third-party libraries

#### libpng

Download the latest version of zlib from https://zlib.net/. Unpack the archive and copy the contents of the top-level directory to "\<root>/Libraries/zlib".

Download the latest version of libpng from http://www.libpng.org/pub/png/libpng.html. Unpack the archive and copy the contents of the top-level directory to "\<root>/Libraries/libpng". Copy the file "\<root>/Libraries/libpng/scripts/pnglibconf.h.prebuilt" to "\<root>/Libraries/libpng/pnglibconf.h".

Open "\<root>/Libraries/libpng/pnglibconf.h" in a text editor, and find and delete all lines that define symbols starting with "PNG_SAVE_", "PNG_SIMPLIFIED_WRITE_" and "PNG_WRITE_".

#### libjpeg

Download libjpeg 6b from http://libjpeg.sourceforge.net/. (Later versions are available, but maintenance of libjpeg seems to have been taken up by someone other than the original authors, who has some distinctly odd ideas.)

Unpack the archive and copy the contents of the "jpeg-6b" directory to "\<root>/Libraries/jpeg". In "\<root>/Libraries/jpeg", rename "jconfig.vc" to "jconfig.h".

Open "\<root>/Libraries/jpeg/jmorecfg.h" in a text editor, and find
```
#ifndef XMD_H			/* X11/xmd.h correctly defines INT32 */
typedef long INT32;
#endif
```
Change it to
```
#if !defined(XMD_H) && !defined(_BASETSD_H_)
typedef long INT32;
#endif
```
In the same file, find
```
#ifdef NEED_FAR_POINTERS
#define FAR  far
#else
#define FAR
#endif
```
Change it to
```
#ifndef FAR
#ifdef NEED_FAR_POINTERS
#define FAR  far
#else
#define FAR
#endif
#endif
```

#### libvorbis

Download the latest stable versions of libogg and libvorbis from https://xiph.org/downloads/. Unpack the libogg archive and copy the contents of the top-level directory to "\<root>/Libraries/libogg". Unpack the libvorbis archive and copy the contents of the top-level directory to "\<root>/Libraries/libvorbis".

### Compiling the project

Start Visual Studio, open the solution "\<root>/Adv/Glk/GlkDll/Glk.sln", then build the "Glk" project.
