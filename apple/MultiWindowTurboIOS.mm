#import <TargetConditionals.h>

#if !TARGET_OS_OSX
#import "MultiWindow.h"
#import "MultiWindowEventEmitter.h"
#import <React/RCTBridgeModule.h>

static const double MWMacOSUnsupportedPlatformCode = -73005.0;
static inline NSNumber *MWMultiply(double a, double b) {
  return @(a * b);
}

@implementation MultiWindow

@synthesize bridge = _bridge;

- (void)setBridge:(RCTBridge *)bridge
{
  _bridge = bridge;
}

- (NSNumber *)multiply:(double)a b:(double)b
{
  NSDictionary *payload = @{
    @"message": [NSString stringWithFormat:@"MultiWindow multiply called with %f and %f", a, b],
    @"a": @(a),
    @"b": @(b),
  };

  MWEmitLogEvent(self.bridge, payload);
  return MWMultiply(a, b);
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<facebook::react::NativeMultiWindowSpecJSI>(params);
}

- (void)openNewWindow:(JS::NativeMultiWindow::WindowOptions &)options
             resolve:(RCTPromiseResolveBlock)resolve
             reject:(RCTPromiseRejectBlock)reject
{
  (void)options;
  (void)reject;

  if (resolve == nil) {
    return;
  }

  resolve(@(MWMacOSUnsupportedPlatformCode));
}

+ (NSString *)moduleName
{
  return @"MultiWindow";
}

@end

#endif
