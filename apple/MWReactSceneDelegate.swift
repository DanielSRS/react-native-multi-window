#if os(iOS)

import UIKit

@available(iOS 13.0, *)
@objcMembers
@objc(MWReactSceneDelegate)
public final class MWReactSceneDelegate: UIResponder, UIWindowSceneDelegate {
  public var window: UIWindow?

  private let host = MWReactNativeHost.shared
  private let configuration = MWReactConfiguration.shared

  public func scene(
    _ scene: UIScene,
    willConnectTo session: UISceneSession,
    options connectionOptions: UIScene.ConnectionOptions
  ) {
    guard let windowScene = scene as? UIWindowScene else {
      return
    }

    let metadata = SceneMetadata(
      connectionOptions: connectionOptions,
      session: session,
      defaultComponentName: configuration.defaultComponentName
    )

    let coordinator = MWIOSSceneCoordinator.shared()

    var pendingRequest: MWIOSPendingWindowRequest?
    if let token = metadata.token {
      pendingRequest = coordinator.consumePendingRequest(withToken: token)
    }

    let window = UIWindow(windowScene: windowScene)
    self.window = window

    host.render(
      moduleName: metadata.componentName,
      in: window,
      initialProperties: configuration.initialProperties(),
      launchOptions: configuration.dequeueLaunchOptions()
    )

    let identifier = coordinator.register(
      window,
      session: session,
      componentName: metadata.componentName,
      title: metadata.title
    )

    if let resolve = pendingRequest?.resolve {
      let result = identifier ?? NSNumber(value: MWIOSWindowErrorCode.embeddingFailed.rawValue)
      resolve(result)
    }

    scene.title = metadata.title
    updateUserActivity(for: scene, metadata: metadata)
  }

  public func sceneDidDisconnect(_ scene: UIScene) {
    MWIOSSceneCoordinator.shared().unregisterSession(scene.session)
  }

  public func stateRestorationActivity(for scene: UIScene) -> NSUserActivity? {
    scene.userActivity
  }

  private func updateUserActivity(for scene: UIScene, metadata: SceneMetadata) {
    guard metadata.shouldPersistUserActivity, let token = metadata.token else {
      scene.userActivity = nil
      return
    }

    let activity = NSUserActivity(activityType: MWIOSSceneActivityType)
    activity.title = metadata.title
    activity.targetContentIdentifier = token
    activity.userInfo = [
      MWIOSSceneTokenKey: token,
      MWIOSSceneComponentNameKey: metadata.componentName,
      MWIOSSceneTitleKey: metadata.title,
      MWIOSSceneIsManagedKey: true,
    ]
    scene.userActivity = activity
  }
}

@available(iOS 13.0, *)
private struct SceneMetadata {
  let token: String?
  let componentName: String
  let title: String
  let shouldPersistUserActivity: Bool

  init(
    connectionOptions: UIScene.ConnectionOptions,
    session: UISceneSession,
    defaultComponentName: String
  ) {
    if let managed = SceneMetadata.metadata(from: connectionOptions)
      ?? SceneMetadata.metadata(from: session.stateRestorationActivity) {
      token = managed.token
      componentName = managed.component
      title = managed.title
      shouldPersistUserActivity = true
    } else {
      token = nil
      componentName = defaultComponentName
      title = defaultComponentName
      shouldPersistUserActivity = false
    }
  }

  private static func metadata(from connectionOptions: UIScene.ConnectionOptions) -> (token: String?, component: String, title: String)? {
    for activity in connectionOptions.userActivities {
      if let managed = metadata(from: activity) {
        return managed
      }
    }
    return nil
  }

  private static func metadata(from activity: NSUserActivity?) -> (token: String?, component: String, title: String)? {
    guard let activity, let userInfo = activity.userInfo else {
      return nil
    }

    guard let isManaged = userInfo[MWIOSSceneIsManagedKey] as? NSNumber, isManaged.boolValue else {
      return nil
    }

    guard let component = userInfo[MWIOSSceneComponentNameKey] as? String else {
      return nil
    }

    let token = userInfo[MWIOSSceneTokenKey] as? String
    let title = (userInfo[MWIOSSceneTitleKey] as? String) ?? component
    return (token, component, title)
  }
}

#endif
