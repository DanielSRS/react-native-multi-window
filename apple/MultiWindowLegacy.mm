#import "MultiWindow.h"
#import <React/RCTBridgeModule.h>
#import "MultiWindowEventEmitter.h"

static inline NSNumber *MWMultiply(double a, double b) {
  return @(a * b);
}

@implementation MultiWindow

@synthesize bridge = _bridge;

RCT_EXPORT_MODULE(MultiWindow)

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(multiply:(double)a b:(double)b)
{
  NSDictionary *payload = @{
    @"message": [NSString stringWithFormat:@"MultiWindow multiply called with %f and %f", a, b],
    @"a": @(a),
    @"b": @(b),
  };

  MWEmitLogEvent(self.bridge, payload);
  return MWMultiply(a, b);
}

@end
