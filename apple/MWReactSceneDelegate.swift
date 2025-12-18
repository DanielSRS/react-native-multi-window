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
    let activity = NSUserActivity(activityType: MWIOSSceneActivityType)
    activity.title = metadata.title
    activity.userInfo = [
      MWIOSSceneTokenKey: metadata.token ?? UUID().uuidString,
      MWIOSSceneComponentNameKey: metadata.componentName,
      MWIOSSceneTitleKey: metadata.title,
    ]
    scene.userActivity = activity
  }
}

@available(iOS 13.0, *)
private struct SceneMetadata {
  let token: String?
  let componentName: String
  let title: String

  init(
    connectionOptions: UIScene.ConnectionOptions,
    session: UISceneSession,
    defaultComponentName: String
  ) {
    if let activity = connectionOptions.userActivities.first ?? session.stateRestorationActivity,
       let userInfo = activity.userInfo,
       let component = userInfo[MWIOSSceneComponentNameKey] as? String {
      token = userInfo[MWIOSSceneTokenKey] as? String
      componentName = component
      title = (userInfo[MWIOSSceneTitleKey] as? String) ?? component
    } else {
      token = nil
      componentName = defaultComponentName
      title = defaultComponentName
    }
  }
}

#endif
