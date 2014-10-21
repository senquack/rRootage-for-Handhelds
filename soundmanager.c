/*
 * $Id: soundmanager.c,v 1.3 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * BGM/SE manager(using SDL_mixer).
 *
 * @version $Revision: 1.3 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "SDL_mixer.h"
#include "soundmanager.h"

//senquack - new gp2x volume control
#if defined(GP2X) || defined(WIZ)
#include 	<sys/ioctl.h>
#include	<sys/soundcard.h>
#include	<unistd.h>
#include	<fcntl.h>
#endif

//senquack - for new settings:
#include "rr.h"

static int useAudio = 0;

#define MUSIC_NUM 3
#define SHARE_LOC "rr_share/"

static char *musicFileName[MUSIC_NUM] = {
//senquack - first ogg file freezes open2x (and maybe other) ARM platforms, changed to wav
#if defined(GP2X) || defined(WIZ)
   "stg_a.wav", "stg_b.ogg", "stg_c.ogg",
#else
   "stg_a.ogg", "stg_b.ogg", "stg_c.ogg",
#endif
};

static Mix_Music *music[MUSIC_NUM];

#define CHUNK_NUM 16

static char *chunkFileName[CHUNK_NUM] = {
   "laser_start.wav", "laser.wav", "damage.wav", "bomb.wav",
   "destroied.wav", "explosion1.wav", "explosion2.wav", "miss.wav",
   "extend.wav",
   "grz.wav", "grzinv.wav",
   "shot.wav", "change.wav",
   "reflec1.wav", "reflec2.wav", "ref_ready.wav",
};

static Mix_Chunk *chunk[CHUNK_NUM];
static int chunkChannel[CHUNK_NUM] = {
   0, 1, 2, 3,
   4, 5, 6, 7, 4,
   6, 7,
   6, 7,
   7, 7, 7,
};

#if defined(GP2X) || defined(WIZ)
//DKS - new
//senquack - for GP2X volume control:
static float gp2x_current_volume = INIT_VOLUME;

static void gp2x_set_volume (int newvol)
{
   printf ("NEW VOLUME:%d\n", newvol);
   fflush (stdout);

   if ((newvol >= 0) && (newvol <= 100)) {
      unsigned long soundDev = open ("/dev/mixer", O_RDWR);
      if (soundDev) {
         int vol = ((newvol << 8) | newvol);
         ioctl (soundDev, SOUND_MIXER_WRITE_PCM, &vol);
         close (soundDev);
      }
   }
}

//senquack - called at startup so we can 
//    set it back to what it was when we exit.  
// Returns 0-100, current mixer volume, -1 on error.
static int gp2x_get_volume (void)
{
   int vol = -1;
   unsigned long soundDev = open ("/dev/mixer", O_RDONLY);
   if (soundDev) {
      ioctl (soundDev, SOUND_MIXER_READ_PCM, &vol);
      close (soundDev);
      if (vol != -1) {
         //just return one channel , not both channels, they're hopefully the same anyways
         return (vol & 0xFF);
      }
   }
   return vol;
}

//senquack
//Will change the volume level up or down, if amount is positive or negative. Will do volume level clamping.
void gp2x_change_volume (float amount)
{
   //senquack - limit amount volume can be changed when we're down low already..
   //    So headphone users can set a good volume
   if ((amount > 0.25f) && gp2x_current_volume <= 20.0) {
      amount = 0.25f;
   } else if ((amount < 0.25f) && gp2x_current_volume <= 20.0) {
      amount = -0.25f;
   }

   gp2x_current_volume += amount;

   if (gp2x_current_volume > 100.0f) {
      gp2x_current_volume = 100.0f;
   } else if (gp2x_current_volume < 0) {
      gp2x_current_volume = 0;
   }
   gp2x_set_volume ((int) gp2x_current_volume);
}
#endif // GP2X volume stuff

void
closeSound ()
{
   int i;
   if (!useAudio)
      return;
   if (Mix_PlayingMusic ()) {
      Mix_HaltMusic ();
   }
   for (i = 0; i < MUSIC_NUM; i++) {
      if (music[i]) {
         Mix_FreeMusic (music[i]);
      }
   }
   for (i = 0; i < CHUNK_NUM; i++) {
      if (chunk[i]) {
         Mix_FreeChunk (chunk[i]);
      }
   }
   Mix_CloseAudio ();
}

// Initialize the sound.

//senquack - trying to fix glibc memory corruption error:
//static void loadSounds() {
//  int i;
//  char name[56];
//
//  for ( i=0 ; i<MUSIC_NUM ; i++ ) {
//    strcpy(name, SHARE_LOC);
//    strcat(name, "sounds/");
//    strcat(name, musicFileName[i]);
//    if ( NULL == (music[i] = Mix_LoadMUS(name)) ) {
//      fprintf(stderr, "Couldn't load: %s\n", name);
//      useAudio = 0;
//      return;
//    }
//  }
//  for ( i=0 ; i<CHUNK_NUM ; i++ ) {
//    strcpy(name, SHARE_LOC);
//    strcat(name, "/sounds/");
//    strcat(name, chunkFileName[i]);
//    if ( NULL == (chunk[i] = Mix_LoadWAV(name)) ) {
//      fprintf(stderr, "Couldn't load: %s\n", name);
//      useAudio = 0;
//      return;
//    }
//  }
//}
static void
loadSounds ()
{
   //senquack
   printf ("entering loadSounds()\n");
   fflush (stdout);

   int i;
   char name[56];

   for (i = 0; i < MUSIC_NUM; i++) {
      strcpy (name, SHARE_LOC);
      strcat (name, "sounds/");
      strcat (name, musicFileName[i]);

      //senquack
      printf ("trying to load %s in loadSounds()\n", name);
      fflush (stdout);

      if (NULL == (music[i] = Mix_LoadMUS (name))) {
//senquack
//      fprintf(stderr, "Couldn't load: %s\n", name);
         printf ("Couldn't load: %s\n", name);
         printf ("Error: %s\n", Mix_GetError ());
         fflush (stdout);
         useAudio = 0;
         return;
      }
   }

   //senquack
   printf ("loaded music in loadSounds()\n");
   fflush (stdout);

   for (i = 0; i < CHUNK_NUM; i++) {
      strcpy (name, SHARE_LOC);
      strcat (name, "/sounds/");
      strcat (name, chunkFileName[i]);
      if (NULL == (chunk[i] = Mix_LoadWAV (name))) {
         fprintf (stderr, "Couldn't load: %s\n", name);
         useAudio = 0;
         return;
      }
   }

   //senquack
   printf ("leaving loadSounds()\n");
   fflush (stdout);
}

void
initSound ()
{
   int audio_rate;
   Uint16 audio_format;
   int audio_channels;
   int audio_buffers;

   if (SDL_InitSubSystem (SDL_INIT_AUDIO) < 0) {
      fprintf (stderr, "Unable to initialize SDL_AUDIO: %s\n",
               SDL_GetError ());
      return;
   }
   //senquack - altering for GP2X: (NOTE: channels = 1 is correct, the original OGG music is mono)
//  audio_rate = 44100;
//  audio_format = AUDIO_S16;
//  audio_channels = 1;
//  audio_buffers = 4096;

//  audio_rate = 22050;
#if defined(GP2X) || defined(WIZ)
   audio_rate = 44100;
   audio_format = AUDIO_S16;
   audio_channels = 1;
   audio_buffers = 512;
   //senquack - MANDATORY TODO - add GCW define to Makefile
#elif defined (GCW)
   audio_rate = 44100;
   audio_format = AUDIO_S16;
   audio_channels = 1;
   audio_buffers = 1024;
#endif

   if (Mix_OpenAudio (audio_rate, audio_format, audio_channels, audio_buffers)
       < 0) {
      fprintf (stderr, "Couldn't open audio: %s\n", SDL_GetError ());
      return;
   } else {
      Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);
   }

#if defined(GP2X) || defined(WIZ)
   //senquack - making sound nicer in Wiz version than GP2X version
   gp2x_set_volume (INIT_VOLUME);
#endif

   useAudio = 1;
   loadSounds ();
}

// Play/Stop the music/chunk.

void
playMusic (int idx)
{
   //senquack
//  if ( !useAudio ) return;
   if (!useAudio || !settings.music)
      return;
   Mix_PlayMusic (music[idx], -1);
}

void
fadeMusic ()
{
   //senquack
//  if ( !useAudio ) return;
   if (!useAudio || !settings.music)
      return;
   Mix_FadeOutMusic (1280);
}

void
stopMusic ()
{
   //senquack
//  if ( !useAudio ) return;
   if (!useAudio || !settings.music)
      return;
   if (Mix_PlayingMusic ()) {
      //senquack - trying to narrow down problem
      Mix_HaltMusic ();
   }
}

void playChunk(int idx) {
  if ( !useAudio ) return;
  Mix_PlayChannel(chunkChannel[idx], chunk[idx], 0);
}
//senquack TODO  - investigate why I said this and then decided not to implement it:
//senquack - new logic to prevent playing of the laser.wav, as it is 
//    barely audible and has annoying clicks in it
//void
//playChunk (int idx)
//{
//   if (!useAudio)
//      return;
////  if ( !useAudio || idx == 1) return;
//   Mix_PlayChannel (chunkChannel[idx], chunk[idx], 0);
//}

void haltChunk(int idx) {
  if ( !useAudio ) return;
  Mix_HaltChannel(chunkChannel[idx]);
}
//senquack TODO  - investigate why I said this and then decided not to implement it:
//senquack - new logic to prevent playing of the laser.wav, as it is 
//    barely audible and has annoying clicks in it
//void
//haltChunk (int idx)
//{
//   if (!useAudio)
//      return;
////  if ( !useAudio || idx == 1) return;
//   Mix_HaltChannel (chunkChannel[idx]);
//}
