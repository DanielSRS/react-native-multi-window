#if os(iOS)

import UIKit
import Foundation
import React
import React_RCTAppDelegate

@objcMembers
public final class MWReactAppIntegration: NSObject {
  /// Registers the given React Native factory with the multi-window host and lets MultiWindow
  /// take over window management.
  @objc(adoptReactNativeFactory:moduleName:initialProperties:launchOptions:window:)
  public static func adoptReactNativeFactory(
    _ factory: RCTReactNativeFactory,
    moduleName: String,
    initialProperties: [AnyHashable: Any]?,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?,
    window: UIWindow?
  ) {
    configure(
      factory: factory,
      moduleName: moduleName,
      initialProperties: initialProperties,
      launchOptions: launchOptions,
      window: window
    )
  }

  private static func configure(
    factory: RCTReactNativeFactory,
    moduleName: String,
    initialProperties: [AnyHashable: Any]?,
    launchOptions: [UIApplication.LaunchOptionsKey: Any]?,
    window: UIWindow?
  ) {
    let configuration = MWReactConfiguration.shared
    configuration.setDefaultComponentName(moduleName)
    configuration.registerLaunchOptions(launchOptions)

    if configuration.initialPropertiesProvider == nil,
       let normalized = normalizedInitialProps(from: initialProperties) {
      configuration.initialPropertiesProvider = { normalized }
    }

    MWReactNativeHost.shared.adoptExternalFactory(factory)

    window?.isHidden = true
    window?.rootViewController = nil
  }

  private static func normalizedInitialProps(from rawProps: [AnyHashable: Any]?) -> [String: Any]? {
    guard let props = rawProps, !props.isEmpty else {
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
}

#endif

