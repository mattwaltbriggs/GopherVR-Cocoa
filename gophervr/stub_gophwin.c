/*
 * stub_gophwin.c
 * Stub implementations for gophwin.c functions.
 * Allows compilation without X11/Motif while building the Cocoa replacement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type definitions to satisfy function signatures */
typedef void *Widget;
typedef void *XtPointer;
typedef int Boolean;

/* Global widget stubs (referenced by other files via extern) */
Widget top = NULL;
Widget Wig_top2 = NULL;

/* Window geometry globals (referenced by other files) */
int menuwin_w = 400;
int menuwin_h = 400;
int menuwin_x = 100;
int menuwin_y = 100;

void
V_GenMenuListWin(void)
{
}

void
V_GenMenuListing(char *aCh_text, int N_pos)
{
    (void)aCh_text; (void)N_pos;
}

void
V_DelMenuListing(void)
{
}

void
V_SetMenuTitle(char *aCh_title)
{
    (void)aCh_title;
}

void
V_GenHistListing(char *aCh_title, char *aCh_url)
{
    (void)aCh_title; (void)aCh_url;
}

void
V_DestroyHistoryData(void)
{
}

void
V_RememberListWinSize(void)
{
}

void
V_CloseListWin(Widget w, XtPointer client_data, XtPointer cbs)
{
    (void)w; (void)client_data; (void)cbs;
}

void
V_ResizeMenuWin(Widget w, XtPointer client_data, void *event, Boolean *crap)
{
    (void)w; (void)client_data; (void)event; (void)crap;
}

void
V_AddStringToList(char *s, int indent)
{
    (void)s; (void)indent;
}

int
mynewsize(int height)
{
    (void)height;
    return 1;
}
