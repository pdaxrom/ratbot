/*
 *  RoboConsol
 *
 *  Copyright (C) 2013 Alexander Chukov <sash@pdaXrom.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <SDL.h>
#include <SDL_rotozoom.h>
#include "lib/aes.h"
#include "lib/tcp.h"
#include "lib/udp.h"
#include "ext/DT_drawtext.h"
#include "commands.h"

static SDL_Surface *mScreen;
static int mFont;
static int mExit;

static const char *mDefaultBotHost = "rk30linaro";
static const int  mDefaultBotPort = 5678;
static tcp_channel *mServer = NULL;

static int mLight1;
static int mLight2;
static int mDirectionCommandsCount;
static int mDirectionCommands[10];
static int mCurrentDirectionCommand;

extern int decompress_jpeg_to_raw(unsigned char *inbuffer, unsigned long inlen, unsigned char **outbuffer, unsigned int *width, unsigned int *height, int *pixelsize);

void exitRequested();
void resetValues();
static void drawTextString(char *str, int x, int y);

//FIXME: jpeg image buffer
static unsigned char *mJpegBuf;

static int openConnection()
{
    mServer = tcp_open(TCP_CLIENT, mDefaultBotHost, mDefaultBotPort);
    if (!mServer) {
	fprintf(stderr, "tcp_open()\n");
	return -1;
    }

    return 0;
}

static void closeConnection()
{
    tcp_close(mServer);
}

static int tcpCheckRecv()
{
    struct timeval tv = {0, 10};
    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(tcp_fd(mServer), &readfs);

    if (select(tcp_fd(mServer) + 1, &readfs, NULL, NULL, &tv) > 0) {
	if (FD_ISSET(tcp_fd(mServer), &readfs)) {
	    char buf[128];
	    int ret;
	    if ((ret = tcp_read(mServer, buf, sizeof(buf))) > 0) {
		if (!strncmp(buf, "jpeg ", 5)) {
		    int len = atoi(buf + 5);
		    fprintf(stderr, "jpeg image len %d\n", len);
		    if (len > 0) {
			if (!mJpegBuf) {
			    // FIXME: calculate it dynamically
			    mJpegBuf = malloc(320*200*4);
			}
			int rdBytes = 0;
			do {
			    ret = tcp_read(mServer, mJpegBuf + rdBytes, len - rdBytes);
			    fprintf(stderr, "received data len %d\n", ret);
			    if (ret <= 0) {
				fprintf(stderr, "error downloading jpeg image\n");
				return 1;
			    }
			    rdBytes += ret;
			} while (rdBytes < len);

			fprintf(stderr, "jpeg image downloaded!\n");

			unsigned char *rawImage;
			unsigned int rawWidth;
			unsigned int rawHeight;
			int rawPixelSize;
			if (decompress_jpeg_to_raw(mJpegBuf, len, &rawImage, &rawWidth, &rawHeight, &rawPixelSize)) {
			    fprintf(stderr, "error decompressing jpeg\n");
			    return 1;
			}
			fprintf(stderr, "preparing image %dx%dx%d\n", rawWidth, rawHeight, rawPixelSize);
//{
//    FILE *f = fopen("image.raw", "wb");
//    if (f) {
//	fwrite(rawImage, 1, rawHeight*rawWidth*rawPixelSize, f);
//	fclose(f);
//    }
//}
			SDL_Surface *img = SDL_CreateRGBSurfaceFrom(rawImage, rawWidth, rawHeight, rawPixelSize * 8, rawWidth * rawPixelSize, 0xff, 0xff00, 0xff0000, 0);
			if (img) {
			    SDL_Surface *tmp = zoomSurface(img, 2, 2, 1);
			    SDL_BlitSurface(tmp, &tmp->clip_rect, mScreen, &mScreen->clip_rect);
			    SDL_FreeSurface(tmp);

//			    SDL_BlitSurface(img, &img->clip_rect, mScreen, &mScreen->clip_rect);
			    SDL_Flip(mScreen);
			    SDL_FreeSurface(img);
			} else {
			    fprintf(stderr, "Can't create surface!\n");
			}
		    }
		}
	    } else {
		fprintf(stderr, "tcp_read() %d\n", ret);
	    }
	}
    }

    return 0;
}

static int tcpSend(char *str)
{
    drawTextString(str, 1, 40);
    if (tcp_write(mServer, str, strlen(str)) != strlen(str)) {
	return 1;
    }

    return 0;
}

static int sendCommand(int cmd)
{
    switch (cmd) {
    case CMD_RESET:
	resetValues();
	return tcpSend("reset");
    case CMD_STOP:
	return tcpSend("stop");
    case CMD_FWD:
	return tcpSend("forward");
    case CMD_REV:
	return tcpSend("backward");
    case CMD_LEFT:
	return tcpSend("left");
    case CMD_RIGHT:
	return tcpSend("right");
    case CMD_LIGHT1:
	if (!mLight1) {
	    mLight1 = 1;
	    return tcpSend("light1on");
	} else {
	    mLight1 = 0;
	    return tcpSend("light1off");
	}
    case CMD_LIGHT2:
	if (!mLight2) {
	    mLight2 = 1;
	    return tcpSend("light2on");
	} else {
	    mLight2 = 0;
	    return tcpSend("light2off");
	}
    case CMD_GETIMAGE:
	return tcpSend("getimage");
    default:
	return 0;
    }
}

static int pushAndSendCommand(int cmd)
{
    if (mDirectionCommandsCount < 2) {
	mDirectionCommands[mDirectionCommandsCount++] = mCurrentDirectionCommand;
	mCurrentDirectionCommand = cmd;
	return sendCommand(cmd);
    } else {
	mDirectionCommandsCount++;
	return 0;
    }
}

static int popAndSendCommand(int cmd)
{
    if (mDirectionCommandsCount > 2) {
	mDirectionCommandsCount--;
	return 0;
    } else {
	mCurrentDirectionCommand = mDirectionCommands[--mDirectionCommandsCount];
	return sendCommand(mCurrentDirectionCommand);
    }
}

static int initScreen(int width, int height)
{
    int sys_flags = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | sys_flags) < 0) {
	fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
	return 1;
    }

    SDL_WM_SetCaption("ROBOLINK CONSOLE", NULL);

    int vid_flags = SDL_HWSURFACE;
    vid_flags |= SDL_DOUBLEBUF;

    mScreen = SDL_SetVideoMode(width, height, 16, vid_flags);

    mFont = DT_LoadFont("ConsoleFont.bmp", 0);

    return 0;
}

static void drawTextString(char *str, int x, int y)
{
//    DT_DrawText(str, mScreen, mFont, x, y);
//    SDL_Flip(mScreen);
}

static void checkInput()
{
    SDL_Event event;

    if(SDL_PollEvent(&event) > 0){
	switch(event.type) {
	    case SDL_QUIT:
		exitRequested();
		break;
	    case SDL_KEYDOWN: {
		    SDLKey sdlkey=event.key.keysym.sym;
		    switch(sdlkey){
		    case SDLK_UP:	pushAndSendCommand(CMD_FWD); break;
		    case SDLK_DOWN:	pushAndSendCommand(CMD_REV); break;
		    case SDLK_LEFT:	pushAndSendCommand(CMD_LEFT); break;
		    case SDLK_RIGHT:	pushAndSendCommand(CMD_RIGHT); break;
		    case SDLK_F1:	sendCommand(CMD_LIGHT1); break;
		    case SDLK_F2:	sendCommand(CMD_LIGHT2); break;
		    }
		}
		break;
	    case SDL_KEYUP: {
		SDLKey sdlkey=event.key.keysym.sym;
		    switch(sdlkey){
		    case SDLK_UP:	popAndSendCommand(CMD_STOP); break;
		    case SDLK_DOWN:	popAndSendCommand(CMD_STOP); break;
		    case SDLK_LEFT:	popAndSendCommand(CMD_STOP); break;
		    case SDLK_RIGHT:	popAndSendCommand(CMD_STOP); break;
		    case SDLK_SPACE:	sendCommand(CMD_GETIMAGE); break;
		    }
		}
		break;
	}
    }
}

void exitRequested()
{
    mExit = 1;
}

void resetValues()
{
    mDirectionCommandsCount = 0;
    mCurrentDirectionCommand = CMD_STOP;
    mLight1 = 0;
    mLight2 = 0;
}

int main(int argc, char *argv[])
{
    if (initScreen(640, 480)) {
	fprintf(stderr, "Can't initialize screen.\n");
	return 1;
    }

    if (openConnection()) {
	fprintf(stderr, "Can't connect to host.\n");
	return 1;
    }

    mJpegBuf = NULL;
    mExit = 0;
    resetValues();

    while (!mExit) {
	checkInput();
	tcpCheckRecv();
    }

    if (mJpegBuf) {
	free(mJpegBuf);
    }

    closeConnection();

    return 0;
}
