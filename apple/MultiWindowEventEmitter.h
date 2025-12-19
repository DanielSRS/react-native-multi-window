#import <React/RCTBridge.h>

static inline void MWEmitLogEvent(RCTBridge *bridge, NSDictionary *payload) {
  if (bridge == nil || payload == nil) {
    return;
  }

  [bridge enqueueJSCall:@"RCTDeviceEventEmitter"
                 method:@"emit"
                   args:@[@"MultiWindow/logs", payload]
             completion:NULL];
}

static inline void MWEmitWindowEvent(RCTBridge *bridge, NSDictionary *payload) {
  if (bridge == nil || payload == nil) {
    return;
  }

  [bridge enqueueJSCall:@"RCTDeviceEventEmitter"
                 method:@"emit"
                   args:@[@"MultiWindow/event", payload]
             completion:NULL];
}
