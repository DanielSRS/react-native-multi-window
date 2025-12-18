#import <TargetConditionals.h>

#if TARGET_OS_OSX && RCT_NEW_ARCH_ENABLED
#import "MultiWindow.h"
#import "MWMacWindowManager.h"
#import "MultiWindowEventEmitter.h"
#import <React/RCTBridgeModule.h>

static inline NSNumber *MWMultiply(double a, double b) {
  return @(a * b);
}

@interface MultiWindow ()
@property (nonatomic, strong) MWMacWindowManager *macWindowManager;
@end

static inline NSDictionary *MWDictionaryFromOptions(JS::NativeMultiWindow::WindowOptions &options) {
  return @{
    @"title": options.title() ?: @"",
    @"componentName": options.componentName() ?: @"",
    @"windows_WindowType": @(options.windows_WindowType()),
  };
}

@implementation MultiWindow

@synthesize bridge = _bridge;
@synthesize macWindowManager = _macWindowManager;

- (void)setBridge:(RCTBridge *)bridge
{
  _bridge = bridge;
  [self.macWindowManager updateBridge:bridge];
}

- (MWMacWindowManager *)macWindowManager
{
  if (_macWindowManager == nil) {
    _macWindowManager = [MWMacWindowManager new];
  }
  return _macWindowManager;
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
  (void)reject;

  if (resolve == nil) {
    return;
  }

  [[self macWindowManager] openNewWindowWithOptions:MWDictionaryFromOptions(options)
                                          completion:^(NSNumber *result) {
                                            resolve(result);
                                          }];
}

+ (NSString *)moduleName
{
  return @"MultiWindow";
}

@end

#endif
