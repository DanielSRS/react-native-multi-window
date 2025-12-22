#import "AppDelegate.h"

#import <React/RCTBundleURLProvider.h>
#import <ReactAppDependencyProvider/RCTAppDependencyProvider.h>
#import <React/RCTRootView.h>

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  self.moduleName = @"MultiWindowExample";
  // You can add your custom initial props in the dictionary below.
  // They will be passed down to the ViewController used by React Native.
  self.initialProps = @{};
  self.dependencyProvider = [RCTAppDependencyProvider new];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(windowDidBecomeKey:)
                                               name:NSWindowDidBecomeKeyNotification
                                             object:nil];
  
  return [super applicationDidFinishLaunching:notification];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
  NSWindow *window = notification.object;
  [window setTitlebarAppearsTransparent:YES];
  [window setStyleMask:[window styleMask] | NSWindowStyleMaskFullSizeContentView];

  window.opaque = NO;
  window.backgroundColor = [NSColor clearColor];
  
  // Add the blur effect
  NSVisualEffectView *blurView = [[NSVisualEffectView alloc] initWithFrame:window.contentView.bounds];
  blurView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
  blurView.material = NSVisualEffectMaterialUnderWindowBackground;
  blurView.state = NSVisualEffectStateActive;

  // Ensure the blur view resizes with the window
  blurView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  
  [window.contentView addSubview:blurView positioned:NSWindowBelow relativeTo:nil];

  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:NSWindowDidBecomeKeyNotification
                                                object:nil];
}

- (NSURL *)sourceURLForBridge:(RCTBridge *)bridge
{
  return [self bundleURL];
}

- (NSURL *)bundleURL
{
#if DEBUG
  return [[RCTBundleURLProvider sharedSettings] jsBundleURLForBundleRoot:@"index"];
#else
  return [[NSBundle mainBundle] URLForResource:@"main" withExtension:@"jsbundle"];
#endif
}

/// This method controls whether the `concurrentRoot`feature of React18 is turned on or off.
///
/// @see: https://reactjs.org/blog/2022/03/29/react-v18.html
/// @note: This requires to be rendering on Fabric (i.e. on the New Architecture).
/// @return: `true` if the `concurrentRoot` feature is enabled. Otherwise, it returns `false`.
- (BOOL)concurrentRootEnabled
{
#ifdef RN_FABRIC_ENABLED
  return true;
#else
  return false;
#endif
}

@end
