/*
 * $Id: soundmanager.h,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * BGM/SE manager header file.
 *
 * @version $Revision: 1.1.1.1 $
 */

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

//senquack - gp2x/wiz volume control
#if defined(GP2X) || defined(WIZ)
void gp2x_change_volume (float amount);
#define INIT_VOLUME	5
#endif


void closeSound ();
void initSound ();
void playMusic (int idx);
void fadeMusic ();
void stopMusic ();
void playChunk (int idx);
void haltChunk (int idx);
#endif // SOUNDMANAGER_H
