#import <TargetConditionals.h>

#if TARGET_OS_OSX

#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>

NS_ASSUME_NONNULL_BEGIN

@interface MWIOSSceneCoordinator : NSObject
+ (instancetype)sharedCoordinator;
@end

NS_ASSUME_NONNULL_END

#else

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <React/RCTBridgeModule.h>

@class RCTBridge;
@class UISceneSession;
@class MWIOSPendingWindowRequest;

NS_ASSUME_NONNULL_BEGIN

extern NSString *const MWIOSSceneActivityType;
extern NSString *const MWIOSSceneTokenKey;
extern NSString *const MWIOSSceneComponentNameKey;
extern NSString *const MWIOSSceneTitleKey;

typedef NS_ENUM(NSInteger, MWIOSWindowErrorCode) {
  MWIOSWindowErrorCodeBridgeUnavailable = -36991,
  MWIOSWindowErrorCodeInvalidComponent = -63912,
  MWIOSWindowErrorCodeEmbeddingFailed = -36853,
  MWIOSWindowErrorCodeManagerUnavailable = -63724,
  MWIOSWindowErrorCodeUnsupportedPlatform = -36755,
  MWIOSWindowErrorCodeSceneRequestFailed = -63576,
  MWIOSWindowErrorCodeCloseCoordinatorUnavailable = -63541,
  MWIOSWindowErrorCodeCloseInvalidIdentifier = -63542,
  MWIOSWindowErrorCodeCloseWindowNotFound = -63543,
  MWIOSWindowErrorCodeCloseRequestFailed = -63544,
};

@interface MWIOSPendingWindowRequest : NSObject
@property (nonatomic, copy) NSString *token;
@property (nonatomic, copy) NSString *componentName;
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy, nullable) RCTPromiseResolveBlock resolve;
@end

@interface MWIOSSceneCoordinator : NSObject

+ (instancetype)sharedCoordinator;

- (void)updateBridge:(RCTBridge *_Nullable)bridge;
- (RCTBridge *_Nullable)bridge;

- (void)requestWindowWithComponent:(NSString *)componentName
                              title:(NSString *)title
                             resolve:(RCTPromiseResolveBlock)resolve;

- (NSNumber *_Nonnull)closeWindowWithIdentifierValue:(double)identifierValue;

- (MWIOSPendingWindowRequest *_Nullable)consumePendingRequestWithToken:(NSString *)token;
- (NSNumber *_Nullable)registerWindow:(UIWindow *)window
                               session:(UISceneSession *)session
                         componentName:(NSString *)componentName
                                 title:(NSString *)title;

- (NSNumber *_Nullable)identifierForSession:(UISceneSession *)session;
- (void)unregisterSession:(UISceneSession *)session;

- (BOOL)canOpenAdditionalScenes;

@end

NS_ASSUME_NONNULL_END

#endif
