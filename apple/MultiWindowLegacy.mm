#import <TargetConditionals.h>

#if TARGET_OS_OSX && !RCT_NEW_ARCH_ENABLED

#import "MultiWindow.h"
#import <React/RCTBridgeModule.h>
#import "MWMacWindowManager.h"

@interface MultiWindow ()
@property (nonatomic, strong) MWMacWindowManager *macWindowManager;
@end

@implementation MultiWindow

@synthesize bridge = _bridge;
@synthesize macWindowManager = _macWindowManager;

RCT_EXPORT_MODULE(MultiWindow)

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

RCT_EXPORT_METHOD(openNewWindow:(NSDictionary *)options
                  resolve:(RCTPromiseResolveBlock)resolve
                  reject:(RCTPromiseRejectBlock)reject)
{
  (void)reject;

  if (resolve == nil) {
    return;
  }

  [[self macWindowManager] openNewWindowWithOptions:options ?: @{}
                                          completion:^(NSNumber *result) {
                                            resolve(result);
                                          }];
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(closeWindowBy:(nonnull NSNumber *)identifier)
{
  NSNumber *result = [[self macWindowManager] closeWindowWithIdentifier:identifier ?: @0];
  return result;
}

@end

#endif
