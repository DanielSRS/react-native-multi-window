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
  MWIOSWindowErrorCodeBridgeUnavailable = -73001,
  MWIOSWindowErrorCodeInvalidComponent = -73002,
  MWIOSWindowErrorCodeEmbeddingFailed = -73003,
  MWIOSWindowErrorCodeManagerUnavailable = -73004,
  MWIOSWindowErrorCodeUnsupportedPlatform = -73005,
  MWIOSWindowErrorCodeSceneRequestFailed = -73006,
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
