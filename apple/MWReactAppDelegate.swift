#if os(iOS)

import UIKit

@objcMembers
@objc(MWReactAppDelegate)
open class MWReactAppDelegate: UIResponder, UIApplicationDelegate {
  public var window: UIWindow?

  private let host = MWReactNativeHost.shared
  private let configuration = MWReactConfiguration.shared

  open var defaultModuleName: String {
    configuration.defaultComponentName
  }

  open func initialProperties() -> [String: Any]? {
    nil
  }

  open var sceneDelegateClass: UIWindowSceneDelegate.Type {
    MWReactSceneDelegate.self
  }

  open func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]? = nil
  ) -> Bool {
    bootstrapConfiguration(launchOptions: launchOptions)
    return true
  }

  open func application(
    _ application: UIApplication,
    configurationForConnecting connectingSceneSession: UISceneSession,
    options: UIScene.ConnectionOptions
  ) -> UISceneConfiguration {
    configuration.sceneConfiguration(for: connectingSceneSession.role)
  }

  private func bootstrapConfiguration(launchOptions: [UIApplication.LaunchOptionsKey: Any]?) {
    configuration.setDefaultComponentName(defaultModuleName)
    configuration.registerLaunchOptions(launchOptions)
    configuration.initialPropertiesProvider = { [weak self] in
      self?.initialProperties()
    }

    configuration.registerSceneDelegateClass(sceneDelegateClass)
  }
}

#endif
