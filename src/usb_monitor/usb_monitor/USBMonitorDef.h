//
// Created by jiangmin on 2017/12/9.
//

#ifndef SRC_USBMONITORDEF_H
#define SRC_USBMONITORDEF_H

enum MonitorFlag{
    
    FILE_ADD = 1,
    FILE_UPDATED,
    FILE_DELETE,
    FILE_OPEN,
    FILE_CLOSE,
    PROC_START,
    PROC_SHUT
	
};

#endif //SRC_USBMONITORDEF_H
