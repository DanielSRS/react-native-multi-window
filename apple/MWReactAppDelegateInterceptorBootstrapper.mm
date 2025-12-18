#import <TargetConditionals.h>

#if TARGET_OS_IOS

#import <Foundation/Foundation.h>

@protocol MWReactAppDelegateInterceptorExports <NSObject>
+ (void)installHooksIfNeeded;
@end

__attribute__((constructor)) static void MWReactAppDelegateInterceptorBootstrap(void)
{
  // Resolve the Swift interceptor dynamically to avoid depending on generated headers.
  Class interceptorClass = NSClassFromString(@"MWReactAppDelegateInterceptor");
  if (interceptorClass && [interceptorClass respondsToSelector:@selector(installHooksIfNeeded)]) {
    [(id<MWReactAppDelegateInterceptorExports>)interceptorClass installHooksIfNeeded];
  }
}

#endif
