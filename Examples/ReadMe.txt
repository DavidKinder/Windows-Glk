
Windows Glk Examples
--------------------

This directory contains three example programs. "Model" and "Multiwin" are
generic Glk example programs written by Andrew Plotkin. "WinGlkStart" is a
Windows Glk specific example, which demonstrates the common requirement of
asking the user for a file name at startup.

If you have Visual C++, load up the solution "Examples.sln", which contains
each example as a separate project.

If you are not using Visual C++, then for each example you need to compile
the following source files:

Model		model.c, winglk_startup.c, Glk.c
Multiwin	multiwin.c, winglk_startup.c, Glk.c
WinGlkStart	WinGlkStart.c, Glk.c

where "Glk.c" is the standard Windows Glk source file in the main Glk
directory. To create executables you will need to link all the source files
in each example together and link along with a suitable import library for
"Glk.dll".

