--- dmenu.c.orig	2022-02-11 11:26:35 UTC
+++ dmenu.c
@@ -10,10 +10,12 @@
 
 #include <X11/Xlib.h>
 #include <X11/Xatom.h>
+#include <X11/Xproto.h>
 #include <X11/Xutil.h>
 #ifdef XINERAMA
 #include <X11/extensions/Xinerama.h>
 #endif
+#include <X11/extensions/Xrender.h>
 #include <X11/Xft/Xft.h>
 
 #include "drw.h"
@@ -25,6 +27,8 @@
 #define LENGTH(X)             (sizeof X / sizeof X[0])
 #define TEXTW(X)              (drw_fontset_getwidth(drw, (X)) + lrpad)
 
+#define OPAQUE                0xffu
+
 /* enums */
 enum { SchemeNorm, SchemeSel, SchemeOut, SchemeLast }; /* color schemes */
 
@@ -53,10 +57,17 @@ static Clr *scheme[SchemeLast];
 static Drw *drw;
 static Clr *scheme[SchemeLast];
 
+static int useargb = 0;
+static Visual *visual;
+static int depth;
+static Colormap cmap;
+
 #include "config.h"
 
-static int (*fstrncmp)(const char *, const char *, size_t) = strncmp;
-static char *(*fstrstr)(const char *, const char *) = strstr;
+static char * cistrstr(const char *s, const char *sub);
+static int (*fstrncmp)(const char *, const char *, size_t) = strncasecmp;
+static char *(*fstrstr)(const char *, const char *) = cistrstr;
+static void xinitvisual();
 
 static void
 appenditem(struct item *item, struct item **list, struct item **last)
@@ -620,7 +631,7 @@ setup(void)
 #endif
 	/* init appearance */
 	for (j = 0; j < SchemeLast; j++)
-		scheme[j] = drw_scm_create(drw, colors[j], 2);
+		scheme[j] = drw_scm_create(drw, colors[j], alphas[i], 2);
 
 	clip = XInternAtom(dpy, "CLIPBOARD",   False);
 	utf8 = XInternAtom(dpy, "UTF8_STRING", False);
@@ -675,11 +686,13 @@ setup(void)
 
 	/* create menu window */
 	swa.override_redirect = True;
-	swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
+	swa.background_pixel = 0;
+	swa.border_pixel = 0;
+	swa.colormap = cmap;
 	swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;
 	win = XCreateWindow(dpy, parentwin, x, y, mw, mh, 0,
-	                    CopyFromParent, CopyFromParent, CopyFromParent,
-	                    CWOverrideRedirect | CWBackPixel | CWEventMask, &swa);
+	                    depth, CopyFromParent, visual,
+	                    CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &swa);
 	XSetClassHint(dpy, win, &ch);
 
 
@@ -727,9 +740,9 @@ main(int argc, char *argv[])
 			topbar = 0;
 		else if (!strcmp(argv[i], "-f"))   /* grabs keyboard before reading stdin */
 			fast = 1;
-		else if (!strcmp(argv[i], "-i")) { /* case-insensitive item matching */
-			fstrncmp = strncasecmp;
-			fstrstr = cistrstr;
+		else if (!strcmp(argv[i], "-s")) { /* case-sensitive item matching */
+			fstrncmp = strncmp;
+			fstrstr = strstr;
 		} else if (i + 1 == argc)
 			usage();
 		/* these options take one argument */
@@ -765,7 +778,8 @@ main(int argc, char *argv[])
 	if (!XGetWindowAttributes(dpy, parentwin, &wa))
 		die("could not get embedding window attributes: 0x%lx",
 		    parentwin);
-	drw = drw_create(dpy, screen, root, wa.width, wa.height);
+	xinitvisual();
+	drw = drw_create(dpy, screen, root, wa.width, wa.height, visual, depth, cmap);
 	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
 		die("no fonts could be loaded.");
 	lrpad = drw->fonts->h;
@@ -786,4 +800,41 @@ main(int argc, char *argv[])
 	run();
 
 	return 1; /* unreachable */
+}
+
+void
+xinitvisual()
+{
+	XVisualInfo *infos;
+	XRenderPictFormat *fmt;
+	int nitems;
+	int i;
+
+	XVisualInfo tpl = {
+		.screen = screen,
+		.depth = 32,
+		.class = TrueColor
+	};
+	long masks = VisualScreenMask | VisualDepthMask | VisualClassMask;
+
+	infos = XGetVisualInfo(dpy, masks, &tpl, &nitems);
+	visual = NULL;
+	for(i = 0; i < nitems; i ++) {
+		fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
+		if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
+			 visual = infos[i].visual;
+			 depth = infos[i].depth;
+			 cmap = XCreateColormap(dpy, root, visual, AllocNone);
+			 useargb = 1;
+			 break;
+		}
+	}
+
+	XFree(infos);
+
+	if (! visual) {
+		visual = DefaultVisual(dpy, screen);
+		depth = DefaultDepth(dpy, screen);
+		cmap = DefaultColormap(dpy, screen);
+	}
 }
