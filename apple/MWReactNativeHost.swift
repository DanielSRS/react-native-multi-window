#if os(iOS)

import UIKit
import React
import React_RCTAppDelegate

@objcMembers
@objc(MWReactNativeHost)
public final class MWReactNativeHost: NSObject {
  public static let shared = MWReactNativeHost()

  private let delegate: MWReactNativeDelegate
  private var factory: RCTReactNativeFactory

  private override init() {
    let delegate = MWReactNativeDelegate()

    if let dependencyProvider = MWReactNativeHost.makeDependencyProvider() {
      delegate.dependencyProvider = dependencyProvider
    }

    self.delegate = delegate
    self.factory = RCTReactNativeFactory(delegate: delegate)
  }

  public func render(
    moduleName: String,
    in window: UIWindow,
    initialProperties: [String: Any]? = nil,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]? = nil
  ) {
    MWReactAppIntegrationGuard.performManagedRender {
      factory.startReactNative(
        withModuleName: moduleName,
        in: window,
        initialProperties: initialProperties,
        launchOptions: launchOptions
      )
    }
  }

  public func adoptExternalFactory(_ factory: RCTReactNativeFactory) {
    self.factory = factory
  }

  private static func makeDependencyProvider() -> RCTDependencyProvider? {
    guard
      let providerClass = NSClassFromString("RCTAppDependencyProvider") as? NSObject.Type,
      let provider = providerClass.init() as? RCTDependencyProvider
    else {
      return nil
    }

    return provider
  }
}

private final class MWReactNativeDelegate: RCTDefaultReactNativeFactoryDelegate {
  override func sourceURL(for bridge: RCTBridge) -> URL? {
    bundleURL()
  }

  override func bundleURL() -> URL? {
#if DEBUG
    return RCTBundleURLProvider.sharedSettings().jsBundleURL(forBundleRoot: "index")
#else
    return Bundle.main.url(forResource: "main", withExtension: "jsbundle")
#endif
  }
}

public typealias ReactNativeHost = MWReactNativeHost

enum MWReactAppIntegrationGuard {
  private static var managedRenderDepth = 0

  static func performManagedRender(_ work: () -> Void) {
    managedRenderDepth += 1
    defer { managedRenderDepth -= 1 }
    work()
  }

  static var isManagedRenderActive: Bool {
    managedRenderDepth > 0
  }
}

#endif
