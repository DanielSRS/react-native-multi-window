#if os(iOS)

import UIKit
import Foundation
import ObjectiveC.runtime
import React
import React_RCTAppDelegate

@objc final class MWReactAppDelegateInterceptor: NSObject {
  private static var hasInstalledHooks = false
  private static let hookLock = NSLock()

  @objc static func installHooksIfNeeded() {
    hookLock.lock()
    defer { hookLock.unlock() }

    guard !hasInstalledHooks else {
      return
    }

    guard MWReactConfiguration.shared.supportsAutomaticAppIntegration else {
      return
    }

    swizzleAppDelegateLifecycle()
    swizzleReactNativeFactory()

    hasInstalledHooks = true
  }

  private static func swizzleAppDelegateLifecycle() {
    guard
      let originalMethod = class_getInstanceMethod(
        RCTAppDelegate.self,
        #selector(UIApplicationDelegate.application(_:didFinishLaunchingWithOptions:))
      ),
      let swizzledMethod = class_getInstanceMethod(
        RCTAppDelegate.self,
        #selector(RCTAppDelegate.mw_multiWindow_application(_:didFinishLaunchingWithOptions:))
      )
    else {
      return
    }

    method_exchangeImplementations(originalMethod, swizzledMethod)
  }

  private static func swizzleReactNativeFactory() {
    swizzleFactorySelector(
      original: #selector(
        RCTReactNativeFactory.startReactNative(
          withModuleName:in:initialProperties:launchOptions:
        )
      ),
      swizzled: #selector(
        RCTReactNativeFactory.mw_multiWindow_startReactNative(
          withModuleName:in:initialProperties:launchOptions:
        )
      )
    )

    swizzleFactorySelector(
      original: #selector(RCTReactNativeFactory.startReactNative(withModuleName:in:launchOptions:)),
      swizzled: #selector(RCTReactNativeFactory.mw_multiWindow_startReactNative(withModuleName:in:launchOptions:))
    )

    swizzleFactorySelector(
      original: #selector(RCTReactNativeFactory.startReactNative(withModuleName:in:)),
      swizzled: #selector(RCTReactNativeFactory.mw_multiWindow_startReactNative(withModuleName:in:))
    )
  }

  private static func swizzleFactorySelector(original: Selector, swizzled: Selector) {
    guard
      let originalMethod = class_getInstanceMethod(RCTReactNativeFactory.self, original),
      let swizzledMethod = class_getInstanceMethod(RCTReactNativeFactory.self, swizzled)
    else {
      return
    }

    method_exchangeImplementations(originalMethod, swizzledMethod)
  }
}

private enum MWReactAppDelegateBridge {
  private static var capturedFactory: RCTReactNativeFactory?

  static func bootstrap(
    from appDelegate: RCTAppDelegate,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) {
    guard configuration.supportsAutomaticAppIntegration else {
      return
    }

    configuration.setDefaultComponentName(appDelegate.moduleName)
    configuration.registerLaunchOptions(launchOptions)
    installInitialPropsProviderIfNeeded(from: appDelegate.initialProps)

    appDelegate.automaticallyLoadReactNativeWindow = false
  }

  static func captureAppDelegateFactoryIfAvailable(_ appDelegate: RCTAppDelegate) {
    guard configuration.supportsAutomaticAppIntegration else {
      return
    }

    let factory = appDelegate.reactNativeFactory

    let alreadyCaptured = capturedFactory === factory
    capturedFactory = factory

    if !alreadyCaptured {
      MWReactNativeHost.shared.adoptExternalFactory(factory)
    }
  }

  static var shouldInterceptFactoryCall: Bool {
    configuration.supportsAutomaticAppIntegration && !MWReactAppIntegrationGuard.isManagedRenderActive
  }

  static func captureFactory(
    factory: RCTReactNativeFactory,
    moduleName: String,
    initialProperties: [AnyHashable: Any]?,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?,
    window: UIWindow?
  ) {
    guard shouldInterceptFactoryCall else {
      return
    }

    configuration.setDefaultComponentName(moduleName)
    configuration.registerLaunchOptions(launchOptions)
    installInitialPropsProviderIfNeeded(from: initialProperties)

    let alreadyCaptured = capturedFactory === factory
    capturedFactory = factory

    if !alreadyCaptured {
      MWReactNativeHost.shared.adoptExternalFactory(factory)
    }

    window?.isHidden = true
    window?.rootViewController = nil
  }

  private static func installInitialPropsProviderIfNeeded(from rawProps: Any?) {
    guard configuration.initialPropertiesProvider == nil else {
      return
    }

    guard let dictionary = normalizedInitialProps(from: rawProps) else {
      return
    }

    configuration.initialPropertiesProvider = { dictionary }
  }

  private static func normalizedInitialProps(from rawProps: Any?) -> [String: Any]? {
    guard let props = rawProps as? [AnyHashable: Any], !props.isEmpty else {
      return nil
    }

    var normalized: [String: Any] = [:]
    var hasEntries = false

    for (key, value) in props {
      guard let keyString = key as? String else {
        continue
      }

      normalized[keyString] = value
      hasEntries = true
    }

    return hasEntries ? normalized : nil
  }

  private static var configuration: MWReactConfiguration {
    MWReactConfiguration.shared
  }
}

extension RCTAppDelegate {
  @objc fileprivate func mw_multiWindow_application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {
    MWReactAppDelegateBridge.bootstrap(from: self, launchOptions: launchOptions)
    let result = mw_multiWindow_application(application, didFinishLaunchingWithOptions: launchOptions)
    MWReactAppDelegateBridge.captureAppDelegateFactoryIfAvailable(self)
    return result
  }
}

extension RCTReactNativeFactory {
  @objc fileprivate func mw_multiWindow_startReactNative(
    withModuleName moduleName: String,
    in window: UIWindow?
  ) {
    if MWReactAppDelegateBridge.shouldInterceptFactoryCall {
      MWReactAppDelegateBridge.captureFactory(
        factory: self,
        moduleName: moduleName,
        initialProperties: nil,
        launchOptions: nil,
        window: window
      )
      return
    }

    mw_multiWindow_startReactNative(withModuleName: moduleName, in: window)
  }

  @objc fileprivate func mw_multiWindow_startReactNative(
    withModuleName moduleName: String,
    in window: UIWindow?,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) {
    if MWReactAppDelegateBridge.shouldInterceptFactoryCall {
      MWReactAppDelegateBridge.captureFactory(
        factory: self,
        moduleName: moduleName,
        initialProperties: nil,
        launchOptions: launchOptions,
        window: window
      )
      return
    }

    mw_multiWindow_startReactNative(
      withModuleName: moduleName,
      in: window,
      launchOptions: launchOptions
    )
  }

  @objc fileprivate func mw_multiWindow_startReactNative(
    withModuleName moduleName: String,
    in window: UIWindow?,
    initialProperties: [AnyHashable: Any]?,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) {
    if MWReactAppDelegateBridge.shouldInterceptFactoryCall {
      MWReactAppDelegateBridge.captureFactory(
        factory: self,
        moduleName: moduleName,
        initialProperties: initialProperties,
        launchOptions: launchOptions,
        window: window
      )
      return
    }

    mw_multiWindow_startReactNative(
      withModuleName: moduleName,
      in: window,
      initialProperties: initialProperties,
      launchOptions: launchOptions
    )
  }
}

#endif
