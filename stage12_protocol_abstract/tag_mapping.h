#ifndef TAG_MAPPING_H
#define TAG_MAPPING_H

#include <QString>

struct TagMapping {
    QString tagName;
    int     area    = 0x84;  // S7AreaDB default
    int     dbNum   = 1;
    int     start   = 0;
    int     size    = 2;
    int     valType = 4;     // 0=BOOL, 1=INT8, 2=UINT8, 3=INT16, 4=UINT16, 5=INT32, 6=FLOAT32
};

#endif
