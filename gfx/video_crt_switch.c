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
#include <retro_common_api.h>
#include "video_crt_switch.h"
#include "video_display_server.h"
#include "../core_info.h"
#include "../verbosity.h"

#ifdef __linux__
#define LIBSWR "libswitchres.so"
#elif _WIN32
#define LIBSWR "libswitchres.dll"
#endif

#include <switchres/switchres_wrapper.h>


static LIBTYPE dlp;
static srAPI* SRobj;
static sr_mode srm;

static int rescheck = 0;
static bool sr2_active = false;
static int super_width                    = 0;

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#if defined(HAVE_VIDEOCORE)
#include "include/userland/interface/vmcs_host/vc_vchi_gencmd.h"
static void crt_rpi_switch(int width, int height, float hz, int xoffset);
#endif

static void switch_crt_hz(videocrt_switch_t *p_switch)
{
 

   /* set hz float to an int for windows switching */
   /*
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
   */
   video_monitor_set_refresh_rate(p_switch->ra_core_hz);

   p_switch->ra_tmp_core_hz = p_switch->ra_core_hz;
}

static void crt_aspect_ratio_switch(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height)
{
   /* send aspect float to video_driver */
   p_switch->fly_aspect = (float)width / (float)height;
   video_driver_set_aspect_ratio_value((float)p_switch->fly_aspect);
   RARCH_LOG("[CRT]: Setting Aspect Ratio: %f \n", (float)p_switch->fly_aspect);
}
/*
static void crt_handheld_fix(videocrt_switch_t *p_switch)
{
   crt_aspect_ratio_switch(p_switch, p_switch->ra_core_width , p_switch->ra_core_height );
   if (p_switch->ra_core_height == 144 && p_switch->ra_set_core_hz == 50)
   {
      p_switch->ra_core_height= 288;
      crt_aspect_ratio_switch(p_switch, p_switch->ra_core_width , p_switch->ra_core_height );
   }else if (p_switch->ra_core_height < 170 )
   {
      p_switch->ra_core_height= 200;
      if (p_switch->ra_core_width > 1000)
         crt_aspect_ratio_switch(p_switch, p_switch->ra_core_width , p_switch->ra_core_height );
   }


}
*/
static void switch_res_crt(
      videocrt_switch_t *p_switch,
      unsigned width, unsigned height, unsigned crt_mode, unsigned native_width)
{
   unsigned char interlace = 0,   ret;
   const char* err_msg;

   #ifdef __linux__
      int w = native_width, h = height;
      super_width = w;
   #elif _WIN32
      int w = native_width, h = height;
      super_width = w;
   #endif
   double rr = p_switch->ra_core_hz;
  /* if (height >= 300 && crt_mode == 1)
      interlace = 1;
   else
      interlace = 0;
   */
   //printf("About to open %s.\n", LIBSWR);
   if (sr2_active == false)
   {
     // Load the lib
      dlp = OPENLIB(LIBSWR);

      /* Loading failed, inform and exit */
      if (!dlp) {
         //printf("Loading %s failed.\n", LIBSWR);
         //printf("Error: %s\n", LIBERROR());
         
      }
      
      // Load the init()
      LIBERROR();
      SRobj =  (srAPI*)LIBFUNC(dlp, "srlib");
      sr2_active = true;

      if ((err_msg = LIBERROR()) != NULL) 
      {
         CLOSELIB(dlp);  
         sr2_active = false;
         RARCH_LOG("[CRT]: Switchres Library failed to load \n");
      }
   
      SRobj->init();
      RARCH_LOG("[CRT]: SR init \n");
      SRobj->sr_init_disp();
      RARCH_LOG("[CRT]: SR init_disp \n");
/*
      #ifdef __linux__
      ret =  SRobj->sr_add_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         SRobj->deinit();
      }
      #endif
      */
      ret =   SRobj->sr_switch_to_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         SRobj->deinit();
      }
   }else{
      /*
      #ifdef __linux__
      ret =  SRobj->sr_add_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         SRobj->deinit();
      }
      #endif
      */
      ret =   SRobj->sr_switch_to_mode(w, h, rr, interlace, &srm);
      if(!ret) 
      {
         SRobj->deinit();
      }
   }
   if (srm.interlace == 0)
      RARCH_LOG("[CRT]: Resolution Set: %dx%d@%f \n", srm.width, srm.height, srm.refresh);
   else
      RARCH_LOG("[CRT]: Resolution Set: %dx%di@%f \n", srm.width, srm.height, srm.refresh);

   //video_monitor_set_refresh_rate(srm.refresh);
   p_switch->ra_core_hz = srm.refresh;

   if (srm.width > width)
   {
      int aw = srm.width/width;
      int bw = width*aw;
      crt_aspect_ratio_switch(p_switch, bw , srm.height);

   }else
      crt_aspect_ratio_switch(p_switch, srm.width , srm.height);

   //video_driver_set_viewport(srm.width , srm.height,0,0);
   //video_driver_set_video_mode(srm.width , srm.height,1);
   video_driver_set_size(srm.width , srm.height);

}

void crt_destroy_modes(videocrt_switch_t *p_switch)
{
   if (sr2_active == true)
   {
      if (SRobj)
      {   
         SRobj->deinit();
         CLOSELIB(dlp);
         RARCH_LOG("[CRT]: SR Destroyed \n");
      }
   }
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
   RARCH_LOG("[CRT]: Dynamic Width Set to: %d \n", dynamic_width );
   return dynamic_width;
}

void crt_switch_res_core(
      videocrt_switch_t *p_switch,
      unsigned native_width, unsigned width, unsigned height,
      float hz, unsigned crt_mode,
      int crt_switch_center_adjust,
      int crt_switch_porch_adjust,
      int monitor_index, bool dynamic)
{
      /* ra_core_hz float passed from within */
    /*  if (width == 4 )
      {
         width = 640;
      }
      if (height == 4 )
      {
         height = 480;
      }
      */
      
      if (height != 4 )
      {
		   p_switch->porch_adjust          = crt_switch_porch_adjust;
         p_switch->ra_core_height        = height;
         p_switch->ra_core_hz            = hz;

         if (dynamic)
            p_switch->ra_core_width      = crt_compute_dynamic_width(p_switch, width);
         else 
            p_switch->ra_core_width      = width;

         p_switch->center_adjust         = crt_switch_center_adjust;
         p_switch->index                 = monitor_index;
/*
         if (crt_mode == 2)
         {
            if (hz > 53)
               p_switch->ra_core_hz      = hz * 2;
            if (hz <= 53)
               p_switch->ra_core_hz      = 120.0f;
         }
 */        
         //crt_handheld_fix(p_switch);
         /* Detect resolution change and switch */
         if ( 
               (p_switch->ra_tmp_height != p_switch->ra_core_height) ||
               (p_switch->ra_core_width != p_switch->ra_tmp_width) || 
               (p_switch->center_adjust != p_switch->tmp_center_adjust||
                p_switch->porch_adjust  !=  p_switch->tmp_porch_adjust )
            )
         {
            #if defined(HAVE_VIDEOCORE)
            if (height > 300)
               height = height/2;
            #endif
            RARCH_LOG("[CRT]: Requested Reolution: %dx%d@%f \n", width, height, hz);
            
            switch_res_crt(p_switch, p_switch->ra_core_width, p_switch->ra_core_height , crt_mode, native_width);
            if (p_switch->ra_core_hz != p_switch->ra_tmp_core_hz)
            {
               switch_crt_hz(p_switch);

            }
            
            video_driver_apply_state_changes();

            p_switch->ra_tmp_height     = p_switch->ra_core_height;
            p_switch->ra_tmp_width      = p_switch->ra_core_width;
            p_switch->tmp_center_adjust = p_switch->center_adjust;
            p_switch->tmp_porch_adjust  = p_switch->porch_adjust;
         }
         /* Check if aspect is correct, if not change */
         if (video_driver_get_aspect_ratio() != p_switch->fly_aspect)
         {
            RARCH_LOG("[CRT]: Setting Aspect Ratio: %f \n", (float)p_switch->fly_aspect);
            video_driver_set_aspect_ratio_value((float)p_switch->fly_aspect);
            video_driver_apply_state_changes();
         }


      }
}
/* only used for RPi3 */
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
