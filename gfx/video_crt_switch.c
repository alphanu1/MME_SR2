/* CRT SwitchRes Core
 * Copyright (C) 2018 Alphanu / Ben Templeman.
 *
 * RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../retroarch.h"
#include "video_crt_switch.h"
#include "video_display_server.h"

#ifdef __linux__
#define LIBSWR "libswitchres.so"
#elif _WIN32
#define LIBSWR "libswitchres.dll"
#endif

#include <switchres/switchres_wrapper.h>

bool sr2_active = false;
LIBTYPE dlp;
srAPI* SRobj;
sr_mode srm;

int rescheck = 0;

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#if defined(HAVE_VIDEOCORE)
#include "include/userland/interface/vmcs_host/vc_vchi_gencmd.h"
static void crt_rpi_switch(int width, int height, float hz, int xoffset);
#endif

static void switch_crt_hz(videocrt_switch_t *p_switch)
{
   float ra_core_hz = p_switch->ra_core_hz;

   /* set hz float to an int for windows switching */
   if (ra_core_hz < 100)
   {
      if (ra_core_hz < 53)
         p_switch->ra_set_core_hz = 50;
      if (ra_core_hz >= 53  &&  ra_core_hz < 57)
         p_switch->ra_set_core_hz = 55;
      if (ra_core_hz >= 57)
         p_switch->ra_set_core_hz = 60;
   }

   if (ra_core_hz > 100)
   {
      if (ra_core_hz < 106)
         p_switch->ra_set_core_hz = 120;
      if (ra_core_hz >= 106  &&  ra_core_hz < 114)
         p_switch->ra_set_core_hz = 110;
      if (ra_core_hz >= 114)
         p_switch->ra_set_core_hz = 120;
   }

   video_monitor_set_refresh_rate(p_switch->ra_set_core_hz);

   p_switch->ra_tmp_core_hz = ra_core_hz;
}

static void crt_aspect_ratio_switch(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height)
{
   /* send aspect float to video_driver */
   p_switch->fly_aspect = (float)width / height;
   video_driver_set_aspect_ratio_value((float)p_switch->fly_aspect);
}

static void switch_res_crt(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height)
{
   unsigned char interlace = 0,   ret;
   const char* err_msg;

   int w = width, h = height;
   double rr = p_switch->ra_core_hz;
   if (height >= 300)
      interlace = 1;
   else
      interlace = 0;

   //printf("About to open %s.\n", LIBSWR);
   if (sr2_active == false)
   {
     // Load the lib
      dlp = OPENLIB(LIBSWR);

      // Loading failed, inform and exit
      if (!dlp) {
         //printf("Loading %s failed.\n", LIBSWR);
         //printf("Error: %s\n", LIBERROR());
         
      }
      


      // Load the init()
      LIBERROR();
      SRobj =  (srAPI*)LIBFUNC(dlp, "srlib");
      sr2_active = true;
      if ((err_msg = LIBERROR()) != NULL) {
            //printf("Failed to load srAPI: %s\n", err_msg);
            CLOSELIB(dlp);
      
            sr2_active = false;
      }
   
            // Testing the function
         //printf("Init a new switchres_manager object:\n");
      SRobj->init();
      SRobj->sr_init_disp();
      //printf("Orignial resolution expected: %dx%d@%f-%d\n", w, h, rr, interlace);

      ret =  SRobj->sr_add_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         //printf("ERROR: couldn't add the required mode. Exiting!\n");
         SRobj->deinit();

      }
      //printf("Got resolution: %dx%d%c@%f\n", srm.width, srm.height, srm.interlace, srm.refresh);
      //printf("Press Any Key to switch to new mode\n");


      ret =   SRobj->sr_switch_to_mode(srm.width, srm.height, rr, srm.interlace, &srm);
      if(!ret) 
      {
         //printf("ERROR: couldn't switch to the required mode. Exiting!\n");
         SRobj->deinit();

         printf("Got resolution: %dx%d%c@%f   %f   %f\n", srm.width, srm.height, srm.interlace, srm.refresh,  p_switch->ra_core_hz,  p_switch->ra_tmp_core_hz);

      }
   }else{
      // Call mode + get result values  


      //printf("Orignial resolution expected: %dx%d@%f-%d\n", w, h, rr, interlace);

      ret =  SRobj->sr_add_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         //printf("ERROR: couldn't add the required mode. Exiting!\n");
         SRobj->deinit();

      }
      printf("Got resolution: %dx%d%c@%f   %f   %f\n", srm.width, srm.height, srm.interlace, srm.refresh,  p_switch->ra_core_hz,  p_switch->ra_tmp_core_hz);
      //video_monitor_set_refresh_rate(srm.refresh);
      
      //printf("Press Any Key to switch to new mode\n");


      ret =   SRobj->sr_switch_to_mode(srm.width, srm.height, rr, srm.interlace, &srm);
      if(!ret) 
      {
         //printf("ERROR: couldn't switch to the required mode. Exiting!\n");
         SRobj->deinit();

      }
      //printf("Got resolution: %dx%d%c@%f\n", srm.width, srm.height, srm.interlace, srm.refresh);
      
   }
   video_monitor_set_refresh_rate(p_switch->ra_core_hz);
   //crt_switch_driver_reinit();
   //srnum++;
   //CLOSELIB(dlp);

   

   /*
   video_display_server_set_resolution(width, height,
         p_switch->ra_set_core_hz,
         p_switch->ra_core_hz,
         p_switch->center_adjust,
         p_switch->index,
         p_switch->center_adjust,
         p_switch->porch_adjust);

#if defined(HAVE_VIDEOCORE)
   crt_rpi_switch(width, height,
         p_switch->ra_core_hz,
         p_switch->center_adjust);
   video_monitor_set_refresh_rate(p_switch->ra_core_hz);
   crt_switch_driver_reinit();
#endif
   video_driver_apply_state_changes();

*/
}

/* Create correct aspect to fit video 
 * if resolution does not exist */

 void crt_destroy_modes(void)
 {
    if (sr2_active == true)
    {
     // SRobj->deinit();
      CLOSELIB(dlp);
    }
 }

static void crt_screen_setup_aspect(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height)
{
#if defined(HAVE_VIDEOCORE)
   if (height > 300)
      height = height/2;
#endif

   if (p_switch->ra_core_hz != p_switch->ra_tmp_core_hz)
      switch_crt_hz(p_switch);

   /* Get original resolution of core */
   if (height == 4)
   {
      /* Detect menu only */
      if (width < 700)
         width = 320;

      height = 240;

      crt_aspect_ratio_switch(p_switch, width, height);
   }

   if (height < 200 && height != 144)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 200;
   }

   if (height > 200)
      crt_aspect_ratio_switch(p_switch, width, height);

   if (height == 144 && p_switch->ra_set_core_hz == 50)
   {
      height = 288;
      crt_aspect_ratio_switch(p_switch, width, height);
   }

   if (height > 200 && height < 224)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 224;
   }

   if (height > 224 && height < 240)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 240;
   }

   if (height > 240 && height < 255)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 254;
   }

   if (height == 528 && p_switch->ra_set_core_hz == 60)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 480;
   }

   if (height >= 240 && height < 255 && p_switch->ra_set_core_hz == 55)
   {
      crt_aspect_ratio_switch(p_switch, width, height);
      height = 254;
   }

   switch_res_crt(p_switch, width, height);
}

static int crt_compute_dynamic_width(
      videocrt_switch_t *p_switch,
      int width)
{
   unsigned i;
   int       dynamic_width     = 0;
   unsigned       min_height   = 261;

#if defined(HAVE_VIDEOCORE)
   p_switch->p_clock           = 32000000;
#else
   p_switch->p_clock           = 21000000;
#endif

   for (i = 0; i < 10; i++)
   {
      dynamic_width = width * i;
      if ((dynamic_width * min_height * p_switch->ra_core_hz) 
            > p_switch->p_clock)
         break;
   }
   return dynamic_width;
}

void crt_switch_res_core(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height,
      float hz, unsigned crt_mode,
      int crt_switch_center_adjust,
      int crt_switch_porch_adjust,
      int monitor_index, bool dynamic)
{
   if (rescheck > 2)
   {
      /* ra_core_hz float passed from within
      * video_driver_monitor_adjust_system_rates() */
      if (width == 4)
      {
         width                        = 320;
         height                       = 240;
      }

      p_switch->porch_adjust          = crt_switch_porch_adjust;
      p_switch->ra_core_height        = height;
      p_switch->ra_core_hz            = hz;

      if (dynamic)
         p_switch->ra_core_width      = crt_compute_dynamic_width(p_switch, width);
      else 
         p_switch->ra_core_width      = width;

      p_switch->center_adjust         = crt_switch_center_adjust;
      p_switch->index                 = monitor_index;

      if (crt_mode == 2)
      {
         if (hz > 53)
            p_switch->ra_core_hz      = hz * 2;
         if (hz <= 53)
            p_switch->ra_core_hz      = 120.0f;
      }

      /* Detect resolution change and switch */
      if (
            (p_switch->ra_tmp_height != p_switch->ra_core_height) ||
            (p_switch->ra_core_width != p_switch->ra_tmp_width) || 
            (p_switch->center_adjust != p_switch->tmp_center_adjust||
            p_switch->porch_adjust  !=  p_switch->tmp_porch_adjust )
         )
         crt_screen_setup_aspect(
               p_switch,
               p_switch->ra_core_width,
               p_switch->ra_core_height);

      p_switch->ra_tmp_height     = p_switch->ra_core_height;
      p_switch->ra_tmp_width      = p_switch->ra_core_width;
      p_switch->tmp_center_adjust = p_switch->center_adjust;
      p_switch->tmp_porch_adjust =  p_switch->porch_adjust;

      /* Check if aspect is correct, if not change */
      if (video_driver_get_aspect_ratio() != p_switch->fly_aspect)
      {
         video_driver_set_aspect_ratio_value((float)p_switch->fly_aspect);
         video_driver_apply_state_changes();
      }
      rescheck = 0;
   }else{
      rescheck++;
   }
}

#if defined(HAVE_VIDEOCORE)
static void crt_rpi_switch(int width, int height, float hz, int xoffset)
{
   char buffer[1024];
   VCHI_INSTANCE_T vchi_instance;
   VCHI_CONNECTION_T *vchi_connection  = NULL;
   static char output[250]             = {0};
   static char output1[250]            = {0};
   static char output2[250]            = {0};
   static char set_hdmi[250]           = {0};
   static char set_hdmi_timing[250]    = {0};
   int i                               = 0;
   int hfp                             = 0;
   int hsp                             = 0;
   int hbp                             = 0;
   int vfp                             = 0;
   int vsp                             = 0;
   int vbp                             = 0;
   int hmax                            = 0;
   int vmax                            = 0;
   int pdefault                        = 8;
   int pwidth                          = 0;
   int ip_flag                         = 0;
   float roundw                        = 0.0f;
   float roundh                        = 0.0f;
   float pixel_clock                   = 0.0f;

   /* set core refresh from hz */
   video_monitor_set_refresh_rate(hz);

   /* following code is the mode line generator */
   hsp    = (width * 0.117) - (xoffset*4);
   if (width < 700)
   {
      hfp    = (width * 0.065);
      hbp  = width * 0.35-hsp-hfp;
   }
   else
   {
      hfp  = (width * 0.033) + (width / 112);
      hbp  = (width * 0.225) + (width /58);
      xoffset = xoffset*2;
   }
   
   hmax = hbp;

   if (height < 241)
      vmax = 261;
   if (height < 241 && hz > 56 && hz < 58)
      vmax = 280;
   if (height < 241 && hz < 55)
      vmax = 313;
   if (height > 250 && height < 260 && hz > 54)
      vmax = 296;
   if (height > 250 && height < 260 && hz > 52 && hz < 54)
      vmax = 285;
   if (height > 250 && height < 260 && hz < 52)
      vmax = 313;
   if (height > 260 && height < 300)
      vmax = 318;

   if (height > 400 && hz > 56)
      vmax = 533;
   if (height > 520 && hz < 57)
      vmax = 580;

   if (height > 300 && hz < 56)
      vmax = 615;
   if (height > 500 && hz < 56)
      vmax = 624;
   if (height > 300)
      pdefault = pdefault * 2;

   vfp = (height + ((vmax - height) / 2) - pdefault) - height;

   if (height < 300)
      vsp = vfp + 3; /* needs to be 3 for progressive */
   if (height > 300)
      vsp = vfp + 6; /* needs to be 6 for interlaced */

   vsp  = 3;
   vbp  = (vmax-height)-vsp-vfp;
   hmax = width+hfp+hsp+hbp;

   if (height < 300)
   {
      pixel_clock = (hmax * vmax * hz) ;
      ip_flag     = 0;
   }

   if (height > 300)
   {
      pixel_clock = (hmax * vmax * (hz/2)) /2 ;
      ip_flag     = 1;
   }
   /* above code is the modeline generator */

   snprintf(set_hdmi_timing, sizeof(set_hdmi_timing),
         "hdmi_timings %d 1 %d %d %d %d 1 %d %d %d 0 0 0 %f %d %f 1 ",
         width, hfp, hsp, hbp, height, vfp,vsp, vbp,
         hz, ip_flag, pixel_clock);

   vcos_init();

   vchi_initialise(&vchi_instance);

   vchi_connect(NULL, 0, vchi_instance);

   vc_vchi_gencmd_init(vchi_instance, &vchi_connection, 1);

   vc_gencmd(buffer, sizeof(buffer), set_hdmi_timing);

   vc_gencmd_stop();

   vchi_disconnect(vchi_instance);

   snprintf(output1,  sizeof(output1),
         "tvservice -e \"DMT 87\" > /dev/null");
   system(output1);
   snprintf(output2,  sizeof(output1),
         "fbset -g %d %d %d %d 24 > /dev/null",
         width, height, width, height);
   system(output2);
}
#endif
