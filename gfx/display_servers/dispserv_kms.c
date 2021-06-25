/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2016-2019 - Brad Parker
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../video_display_server.h"
#include <math.h>

#include <compat/strl.h>
#include <string/stdstring.h>

#include <sys/types.h>
#include <unistd.h>
#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

typedef struct
{
   bool decorations;
   int progress;
   int crt_center;
   unsigned opacity;
   unsigned orig_width;
   unsigned orig_height;
   unsigned orig_refresh;
} dispserv_kms_t;

const video_display_server_t dispserv_kms = {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL, /* set_window_decorations */
   NULL, /* set_resolution */
   NULL, /* get_resolution_list */
   NULL, /* get_output_options */
   NULL,
   NULL, /* get_screen_orientation */
   "kms"
};