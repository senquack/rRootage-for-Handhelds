/*
 * $Id: barragemanager.cc,v 1.4 2003/04/26 03:24:15 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle barrages.
 *
 * @version $Revision: 1.4 $
 */
extern "C"
{
#include "SDL.h"
#include <sys/types.h>
#include <dirent.h>
#include "genmcr.h"
#include "brgmng_mtd.h"
}

#include "barragemanager.h"

Barrage barragePattern[BARRAGE_TYPE_NUM][BARRAGE_PATTERN_MAX];
int barragePatternNum[BARRAGE_TYPE_NUM];

#define SHARE_LOC "rr_share/"
//static const char *BARRAGE_DIR_NAME[BARRAGE_TYPE_NUM] = {
//  "normal", "/usr/share/rRootage/reversible", "/usr/share/rRootage/morph", //"/usr/share/rRootage/simple", "/usr/share/rRootage/morph_heavy", //"/usr/share/rRootage/psy",
//};
static const char *BARRAGE_DIR_NAME[BARRAGE_TYPE_NUM] = {
   "normal", "reversible", "morph", "simple", "morph_heavy", "psy",
};

static int
readBulletMLFiles (const char *dirPath, Barrage brg[])
{
   DIR *dp;
   struct dirent *dir;
   int i = 0;
   char fileName[256];

   char fullDirPath[128];
   strcpy (fullDirPath, SHARE_LOC);
   strcat (fullDirPath, dirPath);

   if ((dp = opendir (fullDirPath)) == NULL) {
      fprintf (stderr, "Can't open directory: %s\n", fullDirPath);
      exit (1);
   }
   while ((dir = readdir (dp)) != NULL) {
      if (strcmp (strrchr (dir->d_name, '.'), ".xml") != 0)
         continue;              // Read .xml files.
      strcpy (fileName, fullDirPath);
      strcat (fileName, "/");
      strncat (fileName, dir->d_name,
               sizeof (fileName) - strlen (fileName) - 1);
      brg[i].bulletml = new BulletMLParserTinyXML (fileName);
      brg[i].bulletml->build ();
      i++;
      printf ("%s\n", fileName);
   }
   closedir (dp);
   return i;
}

void
initBarragemanager ()
{
   for (int i = 0; i < BARRAGE_TYPE_NUM; i++) {
      barragePatternNum[i] =
         readBulletMLFiles (BARRAGE_DIR_NAME[i], barragePattern[i]);
      printf ("--------\n");
   }
}

void
closeBarragemanager ()
{
   for (int i = 0; i < BARRAGE_TYPE_NUM; i++) {
      for (int j = 0; j < barragePatternNum[i]; j++) {
         delete barragePattern[i][j].bulletml;
      }
   }
}
