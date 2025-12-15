#import <TargetConditionals.h>

#if RCT_NEW_ARCH_ENABLED
#import <MultiWindowSpec/MultiWindowSpec.h>
#elif TARGET_OS_OSX
#import <React/RCTBridgeModule.h>
#endif

@interface MultiWindow : NSObject
#if RCT_NEW_ARCH_ENABLED
<NativeMultiWindowSpec>
#elif TARGET_OS_OSX
<RCTBridgeModule>
#endif

@end
