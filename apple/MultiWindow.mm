#import <TargetConditionals.h>

#if TARGET_OS_OSX && !RCT_NEW_ARCH_ENABLED
#import "MultiWindowLegacy.mm"
#else
#import "MultiWindowTurbo.mm"
#endif
