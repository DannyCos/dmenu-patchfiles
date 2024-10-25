--- config.def.h.orig	2022-02-11 11:26:35 UTC
+++ config.def.h
@@ -2,16 +2,23 @@ static int topbar = 1;                      /* -b  opt
 /* Default settings; can be overriden by command line. */
 
 static int topbar = 1;                      /* -b  option; if 0, dmenu appears at bottom     */
+static const unsigned int alpha = 0xD0;     /* Amount of opacity. 0xff is opaque             */
 /* -fn option overrides fonts[0]; default X11 font or font set */
 static const char *fonts[] = {
-	"monospace:size=10"
+	"Hack:style=Regular:size=11"
 };
 static const char *prompt      = NULL;      /* -p  option; prompt to the left of input field */
 static const char *colors[SchemeLast][2] = {
 	/*     fg         bg       */
-	[SchemeNorm] = { "#bbbbbb", "#222222" },
-	[SchemeSel] = { "#eeeeee", "#005577" },
-	[SchemeOut] = { "#000000", "#00ffff" },
+	[SchemeNorm] = { "#cdd6f4", "#45475a" },
+	[SchemeSel] = { "#45475a", "#94e2d5" },
+	[SchemeOut] = { "#000000", "#000000" },
+};
+
+static const unsigned int alphas[SchemeLast][2] = {
+	[SchemeNorm] = { OPAQUE, alpha },
+	[SchemeSel] = { OPAQUE, alpha },
+	[SchemeOut] = { OPAQUE, alpha },
 };
 /* -l option; if nonzero, dmenu uses vertical list with given number of lines */
 static unsigned int lines      = 0;
