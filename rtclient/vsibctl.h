/*========================================================================
 * vsibctl.h  --  VSIB mode settings
 *
 * This contains extern declarations for VSIB mode setting variables.
 *
 *========================================================================*/

#ifndef __VSIBCTL_H
#define __VSIBCTL_H

/*------------------------------------------------------------------------
 * External variables
 *------------------------------------------------------------------------*/

// From vsibctl.c :
extern int vsib_mode;
extern int vsib_mode_gigabit;
extern int vsib_mode_embed_1pps_markers;
extern int vsib_mode_skip_samples;

#endif

/*========================================================================
 * $Log: vsibctl.h,v $
 * Revision 1.1  2006/10/19 07:44:06  jwagnerhki
 * added VSIB config extern vars
 *
 */
