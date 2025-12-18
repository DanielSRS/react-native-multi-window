#import <Foundation/Foundation.h>
#import <TargetConditionals.h>

#if TARGET_OS_OSX
@class RCTBridge;

NS_ASSUME_NONNULL_BEGIN

@interface MWMacWindowManager : NSObject
- (void)updateBridge:(nullable RCTBridge *)bridge;
- (void)openNewWindowWithOptions:(NSDictionary *)options
                       completion:(void (^)(NSNumber *result))completion;
@end

NS_ASSUME_NONNULL_END
#endif
