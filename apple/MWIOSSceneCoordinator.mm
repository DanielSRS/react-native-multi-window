#import <TargetConditionals.h>

#if TARGET_OS_OSX

// macOS does not use the iOS scene coordinator implementation.

#else

#import "MWIOSSceneCoordinator.h"

#import <math.h>
#import <React/RCTBridge.h>
#import "MultiWindowEventEmitter.h"

NSString *const MWIOSSceneActivityType = @"com.reactnativemultiwindow.scene";
NSString *const MWIOSSceneTokenKey = @"token";
NSString *const MWIOSSceneComponentNameKey = @"componentName";
NSString *const MWIOSSceneTitleKey = @"title";

static inline NSNumber *MWWrapIOSError(MWIOSWindowErrorCode code)
{
  return @((double)code);
}

static inline NSString *MWIOSTrimmedString(NSString *value)
{
  if (value == nil) {
    return nil;
  }

  NSString *trimmed = [value stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
  return trimmed.length > 0 ? trimmed : nil;
}

@interface MWIOSWindowEntry : NSObject
@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) UISceneSession *session;
@property (nonatomic, copy) NSString *componentName;
@property (nonatomic, copy) NSString *title;
@end

@implementation MWIOSWindowEntry
@end

@interface MWIOSPendingWindowRequest ()
@end

@implementation MWIOSPendingWindowRequest
@end

@interface MWIOSSceneCoordinator ()
@property (nonatomic, weak) RCTBridge *bridge;
@property (nonatomic, strong) NSMutableDictionary<NSString *, MWIOSPendingWindowRequest *> *pendingRequests;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, MWIOSWindowEntry *> *activeWindows;
@property (nonatomic, strong) NSMapTable<UISceneSession *, NSNumber *> *sessionToIdentifier;
@property (nonatomic, assign) double nextWindowIdentifier;
@end

@implementation MWIOSSceneCoordinator

+ (instancetype)sharedCoordinator
{
  static MWIOSSceneCoordinator *sharedInstance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    sharedInstance = [MWIOSSceneCoordinator new];
  });
  return sharedInstance;
}

- (instancetype)init
{
  self = [super init];
  if (self) {
    _pendingRequests = [NSMutableDictionary dictionary];
    _activeWindows = [NSMutableDictionary dictionary];
    _sessionToIdentifier = [NSMapTable weakToStrongObjectsMapTable];
    _nextWindowIdentifier = 1;
  }
  return self;
}

- (void)updateBridge:(RCTBridge *)bridge
{
  _bridge = bridge;
}

- (RCTBridge *)bridge
{
  return _bridge;
}

- (BOOL)canOpenAdditionalScenes
{
  UIApplication *application = [UIApplication sharedApplication];
  if (!application.supportsMultipleScenes) {
    return NO;
  }
  return UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad;
}

- (void)requestWindowWithComponent:(NSString *)componentName
                              title:(NSString *)title
                             resolve:(RCTPromiseResolveBlock)resolve
{
  if (resolve == nil) {
    return;
  }

  NSString *normalizedComponent = MWIOSTrimmedString(componentName);
  if (normalizedComponent.length == 0) {
    resolve(MWWrapIOSError(MWIOSWindowErrorCodeInvalidComponent));
    return;
  }

  NSString *normalizedTitle = MWIOSTrimmedString(title);
  if (normalizedTitle.length == 0) {
    normalizedTitle = normalizedComponent;
  }

  if (![self canOpenAdditionalScenes]) {
    resolve(MWWrapIOSError(MWIOSWindowErrorCodeUnsupportedPlatform));
    return;
  }

  if (self.bridge == nil) {
    resolve(MWWrapIOSError(MWIOSWindowErrorCodeBridgeUnavailable));
    return;
  }

  NSString *token = [NSUUID UUID].UUIDString;

  MWIOSPendingWindowRequest *request = [MWIOSPendingWindowRequest new];
  request.token = token;
  request.componentName = normalizedComponent;
  request.title = normalizedTitle;
  request.resolve = [resolve copy];

  self.pendingRequests[token] = request;

  NSUserActivity *activity = [[NSUserActivity alloc] initWithActivityType:MWIOSSceneActivityType];
  activity.title = normalizedTitle;
  activity.targetContentIdentifier = token;
  activity.userInfo = @{
    MWIOSSceneTokenKey: token,
    MWIOSSceneComponentNameKey: normalizedComponent,
    MWIOSSceneTitleKey: normalizedTitle,
  };

  UIApplication *application = [UIApplication sharedApplication];
  [application requestSceneSessionActivation:nil
                                userActivity:activity
                                     options:nil
                                errorHandler:^(NSError *error) {
                                  dispatch_async(dispatch_get_main_queue(), ^{
                                    [self failRequestWithToken:token error:error];
                                  });
                                }];
}

- (NSNumber *)closeWindowWithIdentifierValue:(double)identifierValue
{
  if (!isfinite(identifierValue)) {
    return MWWrapIOSError(MWIOSWindowErrorCodeCloseInvalidIdentifier);
  }

  double truncatedIdentifier = floor(identifierValue);
  if (truncatedIdentifier <= 0 || truncatedIdentifier != identifierValue) {
    return MWWrapIOSError(MWIOSWindowErrorCodeCloseInvalidIdentifier);
  }

  NSNumber *identifierKey = @(truncatedIdentifier);
  __block NSNumber *result = nil;

  dispatch_block_t block = ^{
    MWIOSWindowEntry *entry = self.activeWindows[identifierKey];
    if (entry == nil) {
      result = MWWrapIOSError(MWIOSWindowErrorCodeCloseWindowNotFound);
      return;
    }

    UIApplication *application = [UIApplication sharedApplication];
    if (application == nil) {
      result = MWWrapIOSError(MWIOSWindowErrorCodeCloseCoordinatorUnavailable);
      return;
    }

    UISceneSession *session = entry.session;
    if (session == nil) {
      [self.activeWindows removeObjectForKey:identifierKey];
      result = MWWrapIOSError(MWIOSWindowErrorCodeCloseWindowNotFound);
      return;
    }

    [self emitCloseRequestLogForEntry:entry identifier:identifierKey];

    [application requestSceneSessionDestruction:session
                                        options:nil
                                    errorHandler:^(NSError *error) {
                                      [self emitCloseRequestFailureForIdentifier:identifierKey error:error];
                                    }];

    result = identifierKey;
  };

  if ([NSThread isMainThread]) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }

  if (result == nil) {
    result = MWWrapIOSError(MWIOSWindowErrorCodeCloseRequestFailed);
  }

  return result;
}

- (void)failRequestWithToken:(NSString *)token error:(NSError *)error
{
  MWIOSPendingWindowRequest *request = self.pendingRequests[token];
  if (request == nil) {
    return;
  }

  request.resolve(MWWrapIOSError(MWIOSWindowErrorCodeSceneRequestFailed));
  [self.pendingRequests removeObjectForKey:token];
}

- (MWIOSPendingWindowRequest *)consumePendingRequestWithToken:(NSString *)token
{
  MWIOSPendingWindowRequest *request = self.pendingRequests[token];
  if (request != nil) {
    [self.pendingRequests removeObjectForKey:token];
  }
  return request;
}

- (NSNumber *)registerWindow:(UIWindow *)window
                       session:(UISceneSession *)session
                 componentName:(NSString *)componentName
                         title:(NSString *)title
{
  NSNumber *identifier = @(self.nextWindowIdentifier);
  self.nextWindowIdentifier += 1;

  MWIOSWindowEntry *entry = [MWIOSWindowEntry new];
  entry.window = window;
  entry.session = session;
  entry.componentName = componentName ?: @"";
  entry.title = title ?: @"";

  self.activeWindows[identifier] = entry;
  [self.sessionToIdentifier setObject:identifier forKey:session];

  [self emitOpenLogForEntry:entry identifier:identifier];

  return identifier;
}

- (NSNumber *)identifierForSession:(UISceneSession *)session
{
  return [self.sessionToIdentifier objectForKey:session];
}

- (void)unregisterSession:(UISceneSession *)session
{
  NSNumber *identifier = [self identifierForSession:session];
  if (identifier == nil) {
    return;
  }

  MWIOSWindowEntry *entry = self.activeWindows[identifier];
  if (entry != nil) {
    [self emitCloseLogForEntry:entry identifier:identifier];
  }

  [self.activeWindows removeObjectForKey:identifier];
  [self.sessionToIdentifier removeObjectForKey:session];

  [self emitWindowClosedEventForIdentifier:identifier];
}

#pragma mark - Logging

- (void)emitOpenLogForEntry:(MWIOSWindowEntry *)entry identifier:(NSNumber *)identifier
{
  if (self.bridge == nil) {
    return;
  }

  CGRect frame = entry.window.frame;
  NSDictionary *payload = @{
    @"function": @"openNewWindow",
    @"platform": @"ios",
    @"title": entry.title ?: @"",
    @"componentName": entry.componentName ?: @"",
    @"window id": identifier ?: @0,
    @"number of open windows": @(self.activeWindows.count),
    @"isKeyWindow": @(entry.window.isKeyWindow),
    @"screenScale": @(entry.window.screen.scale),
    @"frame": @{
      @"originX": @(CGRectGetMinX(frame)),
      @"originY": @(CGRectGetMinY(frame)),
      @"width": @(CGRectGetWidth(frame)),
      @"height": @(CGRectGetHeight(frame)),
    },
  };

  MWEmitLogEvent(self.bridge, payload);
}

- (void)emitCloseLogForEntry:(MWIOSWindowEntry *)entry identifier:(NSNumber *)identifier
{
  if (self.bridge == nil) {
    return;
  }

  CGRect frame = entry.window.frame;
  NSDictionary *payload = @{
    @"function": @"RemoveWindowCompleted",
    @"window id": identifier ?: @0,
    @"remaining open windows": @(self.activeWindows.count),
    @"frame": @{
      @"originX": @(CGRectGetMinX(frame)),
      @"originY": @(CGRectGetMinY(frame)),
      @"width": @(CGRectGetWidth(frame)),
      @"height": @(CGRectGetHeight(frame)),
    },
    @"componentName": entry.componentName ?: @"",
  };

  MWEmitLogEvent(self.bridge, payload);
}

- (void)emitCloseRequestLogForEntry:(MWIOSWindowEntry *)entry identifier:(NSNumber *)identifier
{
  if (self.bridge == nil) {
    return;
  }

  NSDictionary *payload = @{
    @"function": @"closeWindowBy",
    @"window id": identifier ?: @0,
    @"componentName": entry.componentName ?: @"",
    @"title": entry.title ?: @"",
  };

  MWEmitLogEvent(self.bridge, payload);
}

- (void)emitCloseRequestFailureForIdentifier:(NSNumber *)identifier error:(NSError *)error
{
  if (self.bridge == nil) {
    return;
  }

  NSDictionary *payload = @{
    @"function": @"closeWindowByFailed",
    @"window id": identifier ?: @0,
    @"errorCode": @(error.code),
    @"errorDomain": error.domain ?: @"",
  };

  MWEmitLogEvent(self.bridge, payload);
}

- (void)emitWindowClosedEventForIdentifier:(NSNumber *)identifier
{
  if (self.bridge == nil || identifier == nil) {
    return;
  }

  MWEmitWindowEvent(self.bridge,
                    @{ @"type" : @(764), @"id" : @([identifier doubleValue]) });
}

@end

#endif
