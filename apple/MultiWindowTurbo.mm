#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import "MultiWindowTurboMac.mm"
#else
#import "MultiWindowTurboIOS.mm"
#endif