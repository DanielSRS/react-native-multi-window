#import <TargetConditionals.h>

#if TARGET_OS_OSX && !RCT_NEW_ARCH_ENABLED

#import "MultiWindow.h"
#import <React/RCTBridgeModule.h>

static inline NSNumber *MWMultiply(double a, double b) {
  return @(a * b);
}

@implementation MultiWindow

RCT_EXPORT_MODULE(MultiWindow)

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(multiply:(double)a b:(double)b)
{
  return MWMultiply(a, b);
}

@end

#endif
