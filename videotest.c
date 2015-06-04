/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video deocode demo using OpenMAX IL though the ilcient helper library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

#include "bcm_host.h"
#include "ilclient.h"

#define WIDTH   1920
#define HEIGHT  1080

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif

typedef struct
{
    DISPMANX_DISPLAY_HANDLE_T   display;
    DISPMANX_MODEINFO_T         info;
    void                       *image;
    DISPMANX_UPDATE_HANDLE_T    update;
    DISPMANX_RESOURCE_HANDLE_T  resource;
    DISPMANX_ELEMENT_HANDLE_T   element;
    uint32_t                    vc_image_ptr;

} RECT_VARS_T;

static RECT_VARS_T  gRectVars;

static void FillRect( VC_IMAGE_TYPE_T type, void *image, int pitch, int aligned_height, int x, int y, int w, int h, int val )
{
    int         row;
    int         col;

    uint16_t *line = (uint16_t *)image + y * (pitch>>1) + x;

    for ( row = 0; row < h; row++ )
    {
        for ( col = 0; col < w; col++ )
        {
            line[col] = val;
        }
        line += (pitch>>1);
    }
}

static void init_dispmanx_layer_front(void)
{
    RECT_VARS_T    *vars;
    uint32_t        screen = 0;
    int             ret;
    VC_RECT_T       src_rect;
    VC_RECT_T       dst_rect;
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;
    int width = WIDTH, height = HEIGHT;
    int pitch = ALIGN_UP(width*2, 32);
    int aligned_height = ALIGN_UP(height, 16);
    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             0, /*alpha 0->255*/
                             0 };

    vars = &gRectVars;

    printf("Open display[%i]...\n", screen );
    vars->display = vc_dispmanx_display_open( screen );

    ret = vc_dispmanx_display_get_info( vars->display, &vars->info);
    assert(ret == 0);
    printf( "Display is %d x %d\n", vars->info.width, vars->info.height );

    vars->image = calloc( 1, pitch * height );
    assert(vars->image);

    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0xFFFF );
    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0xF800 );
    vars->resource = vc_dispmanx_resource_create( type,
                                                  width,
                                                  height,
                                                  &vars->vc_image_ptr );
    assert( vars->resource );
    vc_dispmanx_rect_set( &dst_rect, 0, 0, width, height);
    ret = vc_dispmanx_resource_write_data(  vars->resource,
                                            type,
                                            pitch,
                                            vars->image,
                                            &dst_rect );
    assert( ret == 0 );
    vars->update = vc_dispmanx_update_start( 10 );
    assert( vars->update );

    vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16 );

    vc_dispmanx_rect_set( &dst_rect, ( vars->info.width - width ) / 2,
                                     ( vars->info.height - height ) / 2,
                                     width,
                                     height );

    vars->element = vc_dispmanx_element_add(    vars->update,
                                                vars->display,
                                                200,               // layer
                                                &dst_rect,
                                                vars->resource,
                                                &src_rect,
                                                DISPMANX_PROTECTION_NONE,
                                                &alpha,
                                                NULL,             // clamp
                                                VC_IMAGE_ROT0 );

    ret = vc_dispmanx_update_submit_sync( vars->update );
    assert( ret == 0 );

}



static void init_dispmanx_layer_behind(void)
{
    RECT_VARS_T    *vars;
    uint32_t        screen = 0;
    int             ret;
    VC_RECT_T       src_rect;
    VC_RECT_T       dst_rect;
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;
    int width = WIDTH, height = HEIGHT;
    int pitch = ALIGN_UP(width*2, 32);
    int aligned_height = ALIGN_UP(height, 16);
    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             0, /*alpha 0->255*/
                             0 };

    vars = &gRectVars;

    printf("Open display[%i]...\n", screen );
    vars->display = vc_dispmanx_display_open( screen );

    ret = vc_dispmanx_display_get_info( vars->display, &vars->info);
    assert(ret == 0);
    printf( "Display is %d x %d\n", vars->info.width, vars->info.height );

    vars->image = calloc( 1, pitch * height );
    assert(vars->image);

    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0xFFFF );
    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0x007F );
    vars->resource = vc_dispmanx_resource_create( type,
                                                  width,
                                                  height,
                                                  &vars->vc_image_ptr );
    assert( vars->resource );
    vc_dispmanx_rect_set( &dst_rect, 0, 0, width, height);
    ret = vc_dispmanx_resource_write_data(  vars->resource,
                                            type,
                                            pitch,
                                            vars->image,
                                            &dst_rect );
    assert( ret == 0 );
    vars->update = vc_dispmanx_update_start( 10 );
    assert( vars->update );

    vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16 );

    vc_dispmanx_rect_set( &dst_rect, ( vars->info.width - width ) / 2,
                                     ( vars->info.height - height ) / 2,
                                     width,
                                     height );

    vars->element = vc_dispmanx_element_add(    vars->update,
                                                vars->display,
                                                -100,               // layer
                                                &dst_rect,
                                                vars->resource,
                                                &src_rect,
                                                DISPMANX_PROTECTION_NONE,
                                                &alpha,
                                                NULL,             // clamp
                                                VC_IMAGE_ROT0 );

    ret = vc_dispmanx_update_submit_sync( vars->update );
    assert( ret == 0 );

}


static void init_dispmanx_layer_3(void)
{
    RECT_VARS_T    *vars;
    uint32_t        screen = 0;
    int             ret;
    VC_RECT_T       src_rect;
    VC_RECT_T       dst_rect;
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;
    int width = WIDTH, height = HEIGHT;
    int pitch = ALIGN_UP(width*2, 32);
    int aligned_height = ALIGN_UP(height, 16);
    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             0, /*alpha 0->255*/
                             0 };

    vars = &gRectVars;

    printf("Open display[%i]...\n", screen );
    vars->display = vc_dispmanx_display_open( screen );

    ret = vc_dispmanx_display_get_info( vars->display, &vars->info);
    assert(ret == 0);
    printf( "Display is %d x %d\n", vars->info.width, vars->info.height );

    vars->image = calloc( 1, pitch * height );
    assert(vars->image);

    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0xFFFF );
    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0x007F );
    vars->resource = vc_dispmanx_resource_create( type,
                                                  width,
                                                  height,
                                                  &vars->vc_image_ptr );
    assert( vars->resource );
    vc_dispmanx_rect_set( &dst_rect, 0, 0, width, height);
    ret = vc_dispmanx_resource_write_data(  vars->resource,
                                            type,
                                            pitch,
                                            vars->image,
                                            &dst_rect );
    assert( ret == 0 );
    vars->update = vc_dispmanx_update_start( 10 );
    assert( vars->update );

    vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16 );

    vc_dispmanx_rect_set( &dst_rect, ( vars->info.width - width ) / 2,
                                     ( vars->info.height - height ) / 2,
                                     width,
                                     height );

    vars->element = vc_dispmanx_element_add(    vars->update,
                                                vars->display,
                                                -200,               // layer
                                                &dst_rect,
                                                vars->resource,
                                                &src_rect,
                                                DISPMANX_PROTECTION_NONE,
                                                &alpha,
                                                NULL,             // clamp
                                                VC_IMAGE_ROT0 );

    ret = vc_dispmanx_update_submit_sync( vars->update );
    assert( ret == 0 );

}

static void init_dispmanx_layer_4(void)
{
    RECT_VARS_T    *vars;
    uint32_t        screen = 0;
    int             ret;
    VC_RECT_T       src_rect;
    VC_RECT_T       dst_rect;
    VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;
    int width = WIDTH, height = HEIGHT;
    int pitch = ALIGN_UP(width*2, 32);
    int aligned_height = ALIGN_UP(height, 16);
    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             0, /*alpha 0->255*/
                             0 };

    vars = &gRectVars;

    printf("Open display[%i]...\n", screen );
    vars->display = vc_dispmanx_display_open( screen );

    ret = vc_dispmanx_display_get_info( vars->display, &vars->info);
    assert(ret == 0);
    printf( "Display is %d x %d\n", vars->info.width, vars->info.height );

    vars->image = calloc( 1, pitch * height );
    assert(vars->image);

    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0xFFFF );
    FillRect( type, vars->image, pitch, aligned_height,  0,  0, width,      height,      0x007F );
    vars->resource = vc_dispmanx_resource_create( type,
                                                  width,
                                                  height,
                                                  &vars->vc_image_ptr );
    assert( vars->resource );
    vc_dispmanx_rect_set( &dst_rect, 0, 0, width, height);
    ret = vc_dispmanx_resource_write_data(  vars->resource,
                                            type,
                                            pitch,
                                            vars->image,
                                            &dst_rect );
    assert( ret == 0 );
    vars->update = vc_dispmanx_update_start( 10 );
    assert( vars->update );

    vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16 );

    vc_dispmanx_rect_set( &dst_rect, ( vars->info.width - width ) / 2,
                                     ( vars->info.height - height ) / 2,
                                     width,
                                     height );

    vars->element = vc_dispmanx_element_add(    vars->update,
                                                vars->display,
                                                1000,               // layer
                                                &dst_rect,
                                                vars->resource,
                                                &src_rect,
                                                DISPMANX_PROTECTION_NONE,
                                                &alpha,
                                                NULL,             // clamp
                                                VC_IMAGE_ROT0 );

    ret = vc_dispmanx_update_submit_sync( vars->update );
    assert( ret == 0 );

}

static int video_decode_test(char *filename)
{
   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;
   FILE *in;
   int status = 0;
   unsigned int data_len = 0;

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((in = fopen(filename, "rb")) == NULL)
      return -2;

   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      return -3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      return -4;
   }

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
      status = -14;
   list[0] = video_decode;

   // create video_render
   if(status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[1] = video_render;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, video_render, 90);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         data_len += fread(dest, 1, buf->nAllocLen-data_len, in);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to video_render
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            ilclient_change_component_state(video_render, OMX_StateExecuting);
         }
         if(!data_len)
	    fseek(in, 0, SEEK_SET); 
         
         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // wait for EOS from render
      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
                              ILCLIENT_BUFFER_FLAG_EOS, 10000);

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

      ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   }

   fclose(in);

   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_teardown_tunnels(tunnel);

   ilclient_state_transition(list, OMX_StateIdle);
   ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
   return status;
}

int main (int argc, char **argv)
{
   if (argc < 2) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }
   bcm_host_init();
   init_dispmanx_layer_front();
   init_dispmanx_layer_behind();
   init_dispmanx_layer_3();
   init_dispmanx_layer_4();
   return video_decode_test(argv[1]);
}







