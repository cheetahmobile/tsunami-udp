/*========================================================================
 * cvs-buildnr.h  --  Global definition of CVS Check-in Number
 *
 * This file needs to be manually edited (yuck...) every time before you
 * commit something to the CVS.
 *
 * The commit/build number in this file will be printed out by the
 * tsunami applications on start. That allows people to better control
 * which CVS build version they are using since the number is the same
 * on all machines after a 'cvs update' or checkout.
 *
 ========================================================================*/

#ifndef __CVS_BUILDNR_H
#define __CVS_BUILDNR_H

// Build number format:
//   v[ongoing version] [devel/final] cvsbuild [incrementing number]

#define TSUNAMI_CVS_BUILDNR	"v1.1 devel cvsbuild 43"

#endif
