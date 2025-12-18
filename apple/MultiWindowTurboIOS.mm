#import <TargetConditionals.h>

#if !TARGET_OS_OSX
#import "MultiWindow.h"
#import "MultiWindowEventEmitter.h"
#import "MWIOSSceneCoordinator.h"
#import <React/RCTBridgeModule.h>
#import <UIKit/UIKit.h>

static inline NSNumber *MWWrapIOSError(MWIOSWindowErrorCode code)
{
  return @((double)code);
}

static inline NSString *MWTrimmedString(NSString *value)
{
  if (value == nil) {
    return nil;
  }

  NSString *trimmed = [value stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
  return trimmed.length > 0 ? trimmed : nil;
}

@interface MultiWindow ()
@property (nonatomic, strong) MWIOSSceneCoordinator *sceneCoordinator;
@end

@implementation MultiWindow

- (instancetype)init
{
  self = [super init];
  if (self) {
    _sceneCoordinator = [MWIOSSceneCoordinator sharedCoordinator];
  }
  return self;
}

@synthesize bridge = _bridge;

- (void)setBridge:(RCTBridge *)bridge
{
  _bridge = bridge;
  [self.sceneCoordinator updateBridge:bridge];
}

- (NSNumber *)multiply:(double)a b:(double)b
{
  NSDictionary *payload = @{
    @"message": [NSString stringWithFormat:@"MultiWindow multiply called with %f and %f", a, b],
    @"a": @(a),
    @"b": @(b),
  };

  MWEmitLogEvent(self.bridge, payload);
  return @(a * b);
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

  NSString *componentName = MWTrimmedString(options.componentName());
  if (componentName.length == 0) {
    resolve(MWWrapIOSError(MWIOSWindowErrorCodeInvalidComponent));
    return;
  }

  NSString *title = MWTrimmedString(options.title());
  if (title.length == 0) {
    title = componentName;
  }

  if (self.sceneCoordinator == nil) {
    resolve(MWWrapIOSError(MWIOSWindowErrorCodeManagerUnavailable));
    return;
  }

  __weak MultiWindow *weakSelf = self;
  dispatch_block_t requestBlock = ^{
    __strong MultiWindow *strongSelf = weakSelf;
    if (strongSelf == nil) {
      resolve(MWWrapIOSError(MWIOSWindowErrorCodeManagerUnavailable));
      return;
    }

    [strongSelf.sceneCoordinator requestWindowWithComponent:componentName
                                                       title:title
                                                      resolve:^(id result) {
                                                        resolve(result);
                                                      }];
  };

  if ([NSThread isMainThread]) {
    requestBlock();
  } else {
    dispatch_async(dispatch_get_main_queue(), requestBlock);
  }
}

+ (NSString *)moduleName
{
  return @"MultiWindow";
}

@end

#endif
