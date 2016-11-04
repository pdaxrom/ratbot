#ifndef __COMMANDFILE_H__
#define __COMMANDFILE_H__

enum {
    CMD_NONE = 0,
    CMD_RESET,
    CMD_STOP,
    CMD_FWD,
    CMD_REV,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_LIGHT1,
    CMD_LIGHT2,
    CMD_GETIMAGE,
};

typedef struct {
    int use;
    int value;
} __attribute__((packed)) HWEntry;

typedef struct {
    HWEntry leftMotor;
    HWEntry rightMotor;
    HWEntry lampOne;
    HWEntry lampTwo;
} __attribute__((packed)) CommandFile;

#endif
