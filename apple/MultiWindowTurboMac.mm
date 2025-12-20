#import <TargetConditionals.h>

#if TARGET_OS_OSX && RCT_NEW_ARCH_ENABLED
#import "MultiWindow.h"
#import "MWMacWindowManager.h"
#import <React/RCTBridgeModule.h>

@interface MultiWindow ()
@property (nonatomic, strong) MWMacWindowManager *macWindowManager;
@end

static inline NSDictionary *MWDictionaryFromOptions(JS::NativeMultiWindow::WindowOptions &options) {
  NSMutableDictionary *dictionary = [@{
    @"title": options.title() ?: @"",
    @"componentName": options.componentName() ?: @"",
    @"windows_WindowType": @(options.windows_WindowType()),
  } mutableCopy];

  id initialProps = options.initialProps();
  if (initialProps != nil && initialProps != (id)kCFNull) {
    dictionary[@"initialProps"] = initialProps;
  }

  return dictionary;
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

- (double)closeWindowBy:(double)identifier
{
  NSNumber *result = [[self macWindowManager] closeWindowWithIdentifier:@(identifier)];
  return result.doubleValue;
}

+ (NSString *)moduleName
{
  return @"MultiWindow";
}

@end

#endif
