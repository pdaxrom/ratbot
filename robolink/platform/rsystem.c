#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "lib/tcp.h"
#include <linux/videodev2.h>
#include "cam/v4l2uvc.h"

#define BUF_SIZE	256

#define COMMPORT	"/dev/ttyACM0"
#define VIDEODEV	"/dev/video0"
#define VIDEO_WIDTH	320
#define VIDEO_HEIGHT	240

extern int compress_yuyv_to_jpeg (struct vdIn *vd, unsigned char **outbuffer, long unsigned int *outlen, int quality);

static int mCommFd;
static struct vdIn *mVideoIn;

static int openVideo()
{
    int width = VIDEO_WIDTH;
    int height = VIDEO_HEIGHT;
    int format = V4L2_PIX_FMT_YUYV;
    int fps = 2;
    int grabmethod = 1; //mmap
    mVideoIn = (struct vdIn *) malloc (sizeof (struct vdIn));
    if (init_videoIn(mVideoIn, VIDEODEV, width, height, format, fps, grabmethod) < 0) {
	free(mVideoIn);
	mVideoIn = NULL;
	return 1;
    }

    return 0;
}

static void closeVideo()
{
    if (mVideoIn) {
	close_v4l2(mVideoIn);
	free(mVideoIn);
	mVideoIn = NULL;
    }
}

static int sendVideoFrame(tcp_channel *client)
{
    unsigned char *out;
    long unsigned int len;
    char tmp[128];

    if (uvcGrab (mVideoIn) < 0) {
	fprintf (stderr, "Error grabbing\n");
	return 1;
    }

    compress_yuyv_to_jpeg(mVideoIn, &out, &len, 85);

    fprintf(stderr, "jpeg len %ld\n", len);

    snprintf(tmp, sizeof(tmp), "jpeg %ld", len);

    if (tcp_write(client, tmp, sizeof(tmp)) == sizeof(tmp)) {
	if (tcp_write(client, out, len) != len) {
	    fprintf(stderr, "Can't send jpeg!\n");
	}
    }

    // FIXME: fix compressor to reuse cinfo structure
    free(out);

    return 0;
}

static void videoThread(void *arg)
{
    tcp_channel *client = (tcp_channel *) arg;

    while(1) {

	sendVideoFrame(client);
//	if (uvcGrab (mVideoIn) < 0) {
//	    fprintf (stderr, "Error grabbing\n");
//	    continue;
//	}

//	if ((difftime (time (NULL), ref_time) > delay) || delay == 0) {

//	    ref_time = time (NULL);
//	}
    }
}

static int openCommunication()
{
    mCommFd = open(COMMPORT, O_RDWR);
    if (mCommFd < 0) {
	return 1;
    }

    return 0;
}

static void closeCommunication()
{
    close(mCommFd);
}

static int writeString(char *buf)
{
    return write(mCommFd, buf, strlen(buf));
}

static int sendCommand(char *cmd, tcp_channel *client)
{
    if (!strcmp(cmd, "reset")) {
	writeString("CLEAR");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "light1on")) {
	writeString("LIGHT 0 1\n");
	writeString("COMMIT\n");
    } else if (!strcmp(cmd, "light1off")) {
	writeString("LIGHT 0 0\n");
	writeString("COMMIT\n");
    } else if (!strcmp(cmd, "light2on")) {
	writeString("LIGHT 1 1\n");
	writeString("COMMIT\n");
    } else if (!strcmp(cmd, "light2off")) {
	writeString("LIGHT 1 0\n");
	writeString("COMMIT\n");
    } else if (!strcmp(cmd, "stop")) {
	writeString("MOTOR 0 0");
	writeString("MOTOR 1 0");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "forward")) {
	writeString("MOTOR 0 1");
	writeString("MOTOR 1 1");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "backward")) {
	writeString("MOTOR 0 -1");
	writeString("MOTOR 1 -1");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "left")) {
	writeString("MOTOR 0 -1");
	writeString("MOTOR 1 1");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "right")) {
	writeString("MOTOR 0 1");
	writeString("MOTOR 1 -1");
	writeString("COMMIT");
    } else if (!strcmp(cmd, "getimage")) {
	sendVideoFrame(client);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    int r;

    if (openCommunication()) {
	fprintf(stderr, "Can't open communication port %s\n", COMMPORT);
	mCommFd = 1;
//	return -1;
    }

    if (openVideo()) {
	fprintf(stderr, "Can't open webcam %s, disabled!\n", VIDEODEV);
    }

    tcp_channel *server = tcp_open(TCP_SERVER, NULL, 5678);
    if (!server) {
	fprintf(stderr, "tcp_open()\n");
	return -1;
    }

    tcp_channel *client = tcp_accept(server);
    if (!client) {
	fprintf(stderr, "tcp_accept()\n");
	return -1;
    }

    pthread_t tid;
    r = pthread_create(&tid, NULL, (void *) &videoThread, (void *) client);

    while (1) {
	if ((r = tcp_read(client, (uint8_t *)buf, BUF_SIZE)) > 0) {
	    buf[r] = 0;
	    fprintf(stderr, "buf[%d]=%s\n", r, buf);
	    sendCommand(buf, client);
	} else {
	    break;
	}

//    strcpy(buf, "Hello client!");
//    if ((r = tcp_write(client, (uint8_t *)buf, strlen(buf) + 1)) <= 0) {
//	fprintf(stderr, "tcp_write()\n");
//    }

    }

    tcp_close(client);
    tcp_close(server);
    closeVideo();
    closeCommunication();

    return 0;
}
