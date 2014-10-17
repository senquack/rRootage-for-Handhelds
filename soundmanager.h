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

//senquack
#define INIT_VOLUME	5

//senquack - gp2x volume control
//void gp2x_change_volume(int amount);
void gp2x_change_volume (float amount);


void closeSound ();
void initSound ();
void playMusic (int idx);
void fadeMusic ();
void stopMusic ();
void playChunk (int idx);
void haltChunk (int idx);
