--- drw.c.orig	2022-02-11 11:26:35 UTC
+++ drw.c
@@ -61,7 +61,7 @@ Drw *
 }
 
 Drw *
-drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h)
+drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap)
 {
 	Drw *drw = ecalloc(1, sizeof(Drw));
 
@@ -70,8 +70,11 @@ drw_create(Display *dpy, int screen, Window root, unsi
 	drw->root = root;
 	drw->w = w;
 	drw->h = h;
-	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
-	drw->gc = XCreateGC(dpy, root, 0, NULL);
+	drw->visual = visual;
+	drw->depth = depth;
+	drw->cmap = cmap;
+	drw->drawable = XCreatePixmap(dpy, root, w, h, depth);
+	drw->gc = XCreateGC(dpy, drw->drawable, 0, NULL);
 	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
 
 	return drw;
@@ -87,7 +90,7 @@ drw_resize(Drw *drw, unsigned int w, unsigned int h)
 	drw->h = h;
 	if (drw->drawable)
 		XFreePixmap(drw->dpy, drw->drawable);
-	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
+	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, drw->depth);
 }
 
 void
@@ -194,21 +197,22 @@ void
 }
 
 void
-drw_clr_create(Drw *drw, Clr *dest, const char *clrname)
+drw_clr_create(Drw *drw, Clr *dest, const char *clrname, unsigned int alpha)
 {
 	if (!drw || !dest || !clrname)
 		return;
 
-	if (!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen),
-	                       DefaultColormap(drw->dpy, drw->screen),
+	if (!XftColorAllocName(drw->dpy, drw->visual, drw->cmap,
 	                       clrname, dest))
 		die("error, cannot allocate color '%s'", clrname);
+
+	dest->pixel = (dest->pixel & 0x00ffffffU) | (alpha << 24);
 }
 
 /* Wrapper to create color schemes. The caller has to call free(3) on the
  * returned color scheme when done using it. */
 Clr *
-drw_scm_create(Drw *drw, const char *clrnames[], size_t clrcount)
+drw_scm_create(Drw *drw, const char *clrnames[], const unsigned int alphas[], size_t clrcount)
 {
 	size_t i;
 	Clr *ret;
@@ -218,7 +222,7 @@ drw_scm_create(Drw *drw, const char *clrnames[], size_
 		return NULL;
 
 	for (i = 0; i < clrcount; i++)
-		drw_clr_create(drw, &ret[i], clrnames[i]);
+		drw_clr_create(drw, &ret[i], clrnames[i], alphas[i]);
 	return ret;
 }
 
@@ -274,9 +278,7 @@ drw_text(Drw *drw, int x, int y, unsigned int w, unsig
 	} else {
 		XSetForeground(drw->dpy, drw->gc, drw->scheme[invert ? ColFg : ColBg].pixel);
 		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
-		d = XftDrawCreate(drw->dpy, drw->drawable,
-		                  DefaultVisual(drw->dpy, drw->screen),
-		                  DefaultColormap(drw->dpy, drw->screen));
+		d = XftDrawCreate(drw->dpy, drw->drawable, drw->visual, drw->cmap);
 		x += lpad;
 		w -= lpad;
 	}
