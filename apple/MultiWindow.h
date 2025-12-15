#import <TargetConditionals.h>

#if RCT_NEW_ARCH_ENABLED
#import <MultiWindowSpec/MultiWindowSpec.h>
#else
#import <React/RCTBridgeModule.h>
#endif
#import <React/RCTBridge.h>

@interface MultiWindow : NSObject
#if RCT_NEW_ARCH_ENABLED
<NativeMultiWindowSpec>
#else
<RCTBridgeModule>
#endif

@property (nonatomic, weak) RCTBridge *bridge;

@end
