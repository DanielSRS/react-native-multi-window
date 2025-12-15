#import "MultiWindow.h"
#import "MultiWindowEventEmitter.h"

@implementation MultiWindow

@synthesize bridge = _bridge;
- (NSNumber *)multiply:(double)a b:(double)b {
    NSDictionary *payload = @{
        @"message": [NSString stringWithFormat:@"MultiWindow multiply called with %f and %f", a, b],
        @"a": @(a),
        @"b": @(b),
    };

    MWEmitLogEvent(self.bridge, payload);
    NSNumber *result = @(a * b);

    return result;
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    return std::make_shared<facebook::react::NativeMultiWindowSpecJSI>(params);
}

+ (NSString *)moduleName
{
  return @"MultiWindow";
}

@end