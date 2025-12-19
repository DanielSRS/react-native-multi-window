#if os(iOS)

import UIKit

@objcMembers
@objc(MWReactConfiguration)
public final class MWReactConfiguration: NSObject {
  public static let shared = MWReactConfiguration()

  private var explicitDefaultComponentName: String?
  private var storedLaunchOptions: [UIApplication.LaunchOptionsKey: Any]?
  private var registeredSceneDelegateClass: AnyClass = MWReactSceneDelegate.self

  public var initialPropertiesProvider: (() -> [String: Any]?)?

  public var defaultComponentName: String {
    guard let explicitDefaultComponentName, !explicitDefaultComponentName.isEmpty else {
      fatalError("MWReactConfiguration.defaultComponentName accessed before being configured. Call MWReactAppIntegration.adoptReactNativeFactory during application startup.")
    }

    return explicitDefaultComponentName
  }

  public func setDefaultComponentName(_ name: String) {
    explicitDefaultComponentName = name
  }

  public func registerLaunchOptions(_ options: [UIApplication.LaunchOptionsKey: Any]?) {
    storedLaunchOptions = options
  }

  public func dequeueLaunchOptions() -> [UIApplication.LaunchOptionsKey: Any]? {
    defer { storedLaunchOptions = nil }
    return storedLaunchOptions
  }

  public func initialProperties() -> [String: Any]? {
    initialPropertiesProvider?()
  }

  public func registerSceneDelegateClass(_ sceneDelegate: UIWindowSceneDelegate.Type) {
    registeredSceneDelegateClass = sceneDelegate
  }

  public func sceneConfiguration(for role: UISceneSession.Role) -> UISceneConfiguration {
    let configuration = UISceneConfiguration(name: "MultiWindow Default Configuration", sessionRole: role)
    configuration.delegateClass = registeredSceneDelegateClass
    return configuration
  }

}

#endif
