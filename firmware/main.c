/*-------------------------------------------------------------------------
Control firmware for RatBot platform
(c) 2013 Alexander Chukov <sash@pdaXrom.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
-------------------------------------------------------------------------*/

#include <p32xxxx.h>			// always in first place to avoid conflict with const.h ON
#include <typedef.h>			// Pinguino's types definitions
#include <const.h>			// Pinguino's constants definitions
#include <macro.h>			// Pinguino's macros definitions
#include <system.c>			// PIC32 System Core Functions
#include <io.c>				// Pinguino Boards Peripheral Remappage and IOs configurations

#if !defined(__32MX220F032D__) && !defined(__32MX250F128B__) && !defined(__32MX220F032B__)
#include <newlib.c>
#endif

#include <stdarg.h>
#include <delay.c>
#include <printf.c>
#include <cdc.h>

extern unsigned char cdc_trf_state;
#define mUSBUSARTIsTxTrfReady() (cdc_trf_state==0)

#define ATTR __attribute((mips16))
//#define ATTR

#define PROTOCOL_VERSION "1.0"

static const char *ACCEPTED_COMMAND = "ACCEPTED COMMAND";
static const char *REJECTED_COMMAND = "REJECTED COMMAND";

#define MAX_MOTORS	2
#define MAX_LEDS	2

typedef struct {
    short motor[MAX_MOTORS];
    short light[MAX_LEDS];
} HWFILE;

static HWFILE hwFile;
static HWFILE hwTmpFile;

static int ATTR StringToInt(char *string)
{
    int val = 0;
    int sign = 1;
    while (*string) {
	if (*string == '+') {
	    sign = 1;
	} else if (*string == '-') {
	    sign = -1;
	} else if ((*string >= '0') && (*string <= '9')) {
	    val = val * 10 + (*string - '0');
	}
	string++;
    }
    if (sign < 0) {
	return -val;
    } else {
	return val;
    }
}

static void ATTR HwInit(void)
{
    // left motor D7(RC7), D6(RC6)
    TRISCCLR = (1 << 7);
    LATCSET  = (1 << 7);
    TRISCCLR = (1 << 6);
    LATCSET  = (1 << 6);
    // left encoder input A2(RB0), A3(RB1)
    TRISBSET = (1 << 0);
    TRISBSET = (1 << 1);

    // right motor D5(RC5), D3(RC3)
    TRISCCLR = (1 << 5);
    LATCSET  = (1 << 5);
    TRISCCLR = (1 << 3);
    LATCSET  = (1 << 3);
    // right encoder input A4(RB2), A5(RB3)
    TRISBSET = (1 << 2);
    TRISBSET = (1 << 3);

    // led
    TRISACLR = (1 << 10);
    LATACLR  = (1 << 10);
    TRISBCLR = (1 << 15);
    LATBCLR  = (1 << 15);

    // button
    TRISBSET = (1 << 7);
}

static void ATTR LightControl(byte light, byte mode)
{
    if (light == 0) {
	if (mode) {
	    LATASET = (1 << 10);
	} else {
	    LATACLR = (1 << 10);
	}
    } else if (light == 1) {
	if (mode) {
	    LATBSET = (1 << 15);
	} else {
	    LATBCLR = (1 << 15);
	}
    }
}

static void ATTR MotorControl(byte motor, short speed)
{
    // No real speed control yet
    if (motor == 0) {
	if (speed == 0) {
	    LATCSET = (1 << 7);
	    LATCSET = (1 << 6);
	} else if (speed > 0) {
	    LATCSET = (1 << 7);
	    LATCCLR = (1 << 6);
	} else {
	    LATCCLR = (1 << 7);
	    LATCSET = (1 << 6);
	}
    } else if (motor == 1) {
	if (speed == 0) {
	    LATCSET = (1 << 3);
	    LATCSET = (1 << 5);
	} else if (speed > 0) {
	    LATCSET = (1 << 3);
	    LATCCLR = (1 << 5);
	} else {
	    LATCCLR = (1 << 3);
	    LATCSET = (1 << 5);
	}
    }
}

static void ATTR SYS_puts(u8 *buffer, u8 length)
{
    u16 i;
    for (i = 1000; i > 0; --i) {
	if (mUSBUSARTIsTxTrfReady()) {
	    break;
	}
#if defined(__32MX220F032D__)||defined(__32MX250F128B__)||defined(__32MX220F032B__)
	USB_Service();
#else
	CDCTxService();
#endif
    }
    if (i > 0) {
	putUSBUSART(buffer,length);
#if defined(__32MX220F032D__)||defined(__32MX250F128B__)||defined(__32MX220F032B__)
	USB_Service();
#else
	CDCTxService();
#endif
    }
}

static u8 ATTR SYS_gets(u8 *buffer)
{
    u8 numBytesRead;
#if defined(__32MX220F032D__)||defined(__32MX250F128B__)||defined(__32MX220F032B__)
    USB_Service();
    numBytesRead = USB_Service_CDC_GetString( buffer );
#else
    CDCTxService();
    numBytesRead = getsUSBUSART(buffer, 64);
#endif
    return numBytesRead;
}

static void ATTR SYS_printf(const u8 *fmt, ...)
{
    u8 buffer[80];
    u8 length;
    va_list	args;

    va_start(args, fmt);
    length = psprintf2(buffer, fmt, args);
    SYS_puts(buffer, length);
    va_end(args);
}

int ATTR main()
{
#if defined(__32MX220F032D__)||defined(__32MX250F128B__)||defined(__32MX220F032B__)
    SystemConfig(40000000);	// default clock frequency is 40Mhz
#else
    SystemConfig(80000000);	// default clock frequency is 80Mhz
#endif

    IOsetSpecial();
    IOsetDigital();
    IOsetRemap();

    HwInit();

    USBDeviceInit();
#if defined(__32MX220F032D__)||defined(__32MX250F128B__)||defined(__32MX220F032B__)
    // nothing to do
#else
    USBDeviceAttach();
#endif
    Delayms(1500);

    memset((void *)&hwFile, 0, sizeof(hwFile));
    memset((void *)&hwTmpFile, 0, sizeof(hwTmpFile));

    while (1) {
	char buf[257];
	int  length = SYS_gets(buf);

	if (length > 0) {
	    char *argv[4];
	    int   argc;
	    char *ptrBegin;
	    char *ptrEnd;

	    buf[length] = 0;
	    while ((buf[length - 1] <= 32) && (length > 0)) {
		buf[length - 1] = 0;
		length--;
	    }

	    argc = 0;
	    ptrBegin = buf;
	    ptrEnd = ptrBegin;
	    while (1) {
		if (*ptrBegin == ' ') {
		    ptrBegin++;
		    ptrEnd++;
		    continue;
		}
		if ((*ptrEnd == ' ') || (*ptrEnd == 0)) {
		    if (*ptrBegin) {
			argv[argc++] = ptrBegin;
		    }
		    if (*ptrEnd == 0) {
			break;
		    }
		    *ptrEnd = 0;
		    if (argc >= sizeof(argv) / sizeof(char *)) {
			break;
		    }
		    ptrBegin = ++ptrEnd;
		    continue;
		}
		ptrEnd++;
	    }

	    if (argc > 0) {
		if (!strcmp(argv[0], "LIGHT")) {
		    if (argc > 2) {
			int number = StringToInt(argv[1]);
			int value = StringToInt(argv[2]);
			if (number < sizeof(hwFile.light) / sizeof(hwFile.light[0])) {
			    //SYS_printf("light[%d]=%d\n\r", number, value);
			    hwTmpFile.light[number] = value;
			}
		    }
		} else if (!strcmp(argv[0], "MOTOR")) {
		    if (argc > 2) {
			int number = StringToInt(argv[1]);
			int value = StringToInt(argv[2]);
			if (number < sizeof(hwFile.motor) / sizeof(hwFile.motor[0])) {
			    //SYS_printf("motor[%d]=%d\n\r", number, value);
			    hwTmpFile.motor[number] = value;
			}
		    }
		} else if (!strcmp(argv[0], "CLEAR")) {
		    memset((void *)&hwTmpFile, 0, sizeof(hwTmpFile));
		} else if (!strcmp(argv[0], "COMMIT")) {
		    int i;
		    for (i = 0; i < (sizeof(hwFile.light) / sizeof(hwFile.light[0])); i++) {
			if (hwFile.light[i] != hwTmpFile.light[i]) {
			    hwFile.light[i] = hwTmpFile.light[i];
			    LightControl(i, hwFile.light[i]);
			}
		    }
		    for (i = 0; i < (sizeof(hwFile.motor) / sizeof(hwFile.motor[0])); i++) {
			if (hwFile.motor[i] != hwTmpFile.motor[i]) {
			    hwFile.motor[i] = hwTmpFile.motor[i];
			    MotorControl(i, hwFile.motor[i]);
			}
		    }
		} else if (!strcmp(argv[0], "INFO")) {
		    SYS_printf("%s %s PROTOCOL VERSION %s\n\r",
				ACCEPTED_COMMAND, argv[0], PROTOCOL_VERSION);
		    SYS_printf("%s %s SYSCLK %d\n\r",
				ACCEPTED_COMMAND, argv[0], GetSystemClock());
		    SYS_printf("%s %s .\n\r",
				ACCEPTED_COMMAND, argv[0]);
		} else {
		    SYS_printf("%s %s UNKNOWN COMMAND\n\r%s %s .\n\r",
				REJECTED_COMMAND, argv[0],
				REJECTED_COMMAND, argv[0]);
		}
	    }
	}
    }

    return(0);
}

void Serial1Interrupt(void)
{
    Nop();
}

void Serial2Interrupt(void)
{
    Nop();
}

void Tmr2Interrupt(void)
{
    Nop();
}

void RTCCInterrupt(void)
{
    Nop();
}
