#if os(iOS)

import UIKit

@objcMembers
@objc(MWReactConfiguration)
public final class MWReactConfiguration: NSObject {
  public static let shared = MWReactConfiguration()

  private let defaultComponentInfoKey = "MultiWindowDefaultComponentName"
  private let sceneManifestKey = "UIApplicationSceneManifest"
  private let sceneConfigurationsKey = "UISceneConfigurations"
  private let sceneDelegateClassKey = "UISceneDelegateClassName"
  private let inspectedSceneRoles = [
    "UIWindowSceneSessionRoleApplication",
    "UIWindowSceneSessionRoleExternalDisplay",
    "UIWindowSceneSessionRoleCarTemplateApplication",
  ]

  private var explicitDefaultComponentName: String?
  private var cachedInfoComponentName: String?
  private var storedLaunchOptions: [UIApplication.LaunchOptionsKey: Any]?
  private var registeredSceneDelegateClass: AnyClass = MWReactSceneDelegate.self
  private var cachedManifestSceneDelegatePresence: Bool?

  public var initialPropertiesProvider: (() -> [String: Any]?)?

  public var defaultComponentName: String {
    if let explicitDefaultComponentName, !explicitDefaultComponentName.isEmpty {
      return explicitDefaultComponentName
    }

    if cachedInfoComponentName == nil {
      cachedInfoComponentName = infoPlistDefaultComponentName()
    }

    if let cachedInfoComponentName, !cachedInfoComponentName.isEmpty {
      return cachedInfoComponentName
    }

    return "App"
  }

  public func setDefaultComponentName(_ name: String?) {
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

  public var supportsAutomaticAppIntegration: Bool {
    if let cachedManifestSceneDelegatePresence {
      return cachedManifestSceneDelegatePresence
    }

    let resolvedPresence = sceneManifestReferencesModuleSceneDelegate()
    cachedManifestSceneDelegatePresence = resolvedPresence
    return resolvedPresence
  }

  private func infoPlistDefaultComponentName() -> String? {
    if let preferredName = Bundle.main.object(forInfoDictionaryKey: defaultComponentInfoKey) as? String,
       !preferredName.isEmpty {
      return preferredName
    }

    if let bundleName = Bundle.main.object(forInfoDictionaryKey: "CFBundleName") as? String,
       !bundleName.isEmpty {
      return bundleName
    }

    return nil
  }

  private func sceneManifestReferencesModuleSceneDelegate() -> Bool {
    guard
      let manifest = Bundle.main.object(forInfoDictionaryKey: sceneManifestKey) as? [String: Any],
      let configurations = manifest[sceneConfigurationsKey] as? [String: Any]
    else {
      return false
    }

    for role in inspectedSceneRoles {
      guard let entries = configurations[role] as? [[String: Any]] else {
        continue
      }

      for entry in entries {
        if let delegateName = entry[sceneDelegateClassKey] as? String,
           delegateName.hasSuffix("MWReactSceneDelegate") {
          return true
        }
      }
    }

    return false
  }
}

#endif
