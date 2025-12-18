#if os(macOS)

import AppKit
import React

@objcMembers
@objc(MWMacWindowManager)
final class MWMacWindowManager: NSObject, NSWindowDelegate {
  @objc enum ErrorCode: Int {
    case bridgeUnavailable = -73001
    case invalidComponentName = -73002
    case viewEmbeddingFailed = -73003
    case managerUnavailable = -73004
  }

  private struct ParsedOptions {
    let componentName: String
    let title: String
  }

  private struct ManagedWindow {
    let id: UInt64
    let window: NSWindow
  }

  private weak var bridge: RCTBridge?
  private var nextWindowIdentifier: UInt64 = 1
  private var openWindows: [UInt64: ManagedWindow] = [:]

  func updateBridge(_ bridge: RCTBridge?) {
    self.bridge = bridge
  }

  func openNewWindow(withOptions options: NSDictionary?, completion: @escaping (NSNumber) -> Void) {
    guard let bridge else {
      completion(Self.wrapError(.bridgeUnavailable))
      return
    }

    guard let parsedOptions = Self.parsedOptions(from: options) else {
      completion(Self.wrapError(.invalidComponentName))
      return
    }

    let executeWindowCreation = { [weak self] in
      guard let self else {
        completion(Self.wrapError(.managerUnavailable))
        return
      }

      let window = self.buildWindow(title: parsedOptions.title)

      guard self.embedReactView(component: parsedOptions.componentName, bridge: bridge, into: window) else {
        completion(Self.wrapError(.viewEmbeddingFailed))
        return
      }

      let windowIdentifier = self.nextWindowIdentifier
      self.nextWindowIdentifier += 1

      window.identifier = NSUserInterfaceItemIdentifier("multiwindow-\(windowIdentifier)")

      self.openWindows[windowIdentifier] = ManagedWindow(id: windowIdentifier, window: window)
      self.emitOpenLog(options: parsedOptions, window: window)

      window.makeKeyAndOrderFront(nil)
      if !NSApplication.shared.isActive {
        NSApplication.shared.activate(ignoringOtherApps: true)
      }

      completion(NSNumber(value: Double(windowIdentifier)))
    }

    if Thread.isMainThread {
      executeWindowCreation()
    } else {
      DispatchQueue.main.async(execute: executeWindowCreation)
    }
  }

  func windowWillClose(_ notification: Notification) {
    guard let window = notification.object as? NSWindow else {
      return
    }

    guard let entry = openWindows.first(where: { $0.value.window === window }) else {
      return
    }

    let windowId = entry.key

    DispatchQueue.main.async { [weak self, window] in
      guard let self else {
        return
      }

      self.openWindows.removeValue(forKey: windowId)
      window.delegate = nil

      let frame = window.frame
      self.emitLog([
        "function": "RemoveWindowCompleted",
        "remaining open windows": Double(self.openWindows.count),
        "window id": Double(windowId),
        "frame": [
          "originX": Double(frame.origin.x),
          "originY": Double(frame.origin.y),
          "width": Double(frame.size.width),
          "height": Double(frame.size.height),
        ],
        "isVisible": window.isVisible,
        "isKeyWindow": window.isKeyWindow,
        "isMiniaturized": window.isMiniaturized,
        "screenName": window.screen?.localizedName ?? "unknown",
      ])
    }
  }

  private func buildWindow(title: String) -> NSWindow {
    var cascadePoint = NSPoint(x: 30, y: 50)
    let windowRect = focusedWindowFrame() ?? NSRect(x: 100, y: 100, width: 600, height: 400)
    let contentRect = NSWindow.contentRect(
      forFrameRect: windowRect,
      styleMask: [.titled, .closable, .miniaturizable, .resizable]
    )

    let window = NSWindow(
      contentRect: contentRect,
      styleMask: [.titled, .closable, .miniaturizable, .resizable],
      backing: .buffered,
      defer: false
    )
    cascadePoint = window.cascadeTopLeft(from: cascadePoint)
    window.isReleasedWhenClosed = false
    window.title = title
    window.delegate = self

    return window
  }

  private func focusedWindowFrame() -> NSRect? {
    if let keyFrame = NSApplication.shared.keyWindow?.frame {
      return keyFrame
    }
    return NSApplication.shared.mainWindow?.frame
  }

  private func embedReactView(component: String, bridge: RCTBridge, into window: NSWindow) -> Bool {
    let rootView = RCTRootView(bridge: bridge, moduleName: component, initialProperties: nil)
    guard let contentView = window.contentView else {
      return false
    }

    rootView.frame = contentView.bounds
    rootView.autoresizingMask = [.width, .height]
    contentView.addSubview(rootView)

    return true
  }

  private func emitOpenLog(options: ParsedOptions, window: NSWindow) {
    let frame = window.frame
    emitLog([
      "function": "openNewWindow",
      "title": options.title,
      "componentName": options.componentName,
      "number of open windows": Double(openWindows.count),
      "frame": [
        "originX": Double(frame.origin.x),
        "originY": Double(frame.origin.y),
        "width": Double(frame.size.width),
        "height": Double(frame.size.height),
      ],
      "isVisible": window.isVisible,
      "isKeyWindow": window.isKeyWindow,
      "isMiniaturized": window.isMiniaturized,
      "screenName": window.screen?.localizedName ?? "unknown",
    ])
  }

  private func emitLog(_ payload: [String: Any]) {
    guard let bridge else {
      return
    }

    bridge.enqueueJSCall(
      "RCTDeviceEventEmitter",
      method: "emit",
      args: ["MultiWindow/logs", payload],
      completion: nil
    )
  }

  private static func parsedOptions(from options: NSDictionary?) -> ParsedOptions? {
    guard let componentName = trimmedString(from: options?["componentName"]) else {
      return nil
    }

    let title = trimmedString(from: options?["title"]) ?? componentName

    return ParsedOptions(componentName: componentName, title: title)
  }

  private static func trimmedString(from value: Any?) -> String? {
    guard let stringValue = value as? String else {
      return nil
    }

    let trimmed = stringValue.trimmingCharacters(in: .whitespacesAndNewlines)
    return trimmed.isEmpty ? nil : trimmed
  }

  private static func wrapError(_ code: ErrorCode) -> NSNumber {
    NSNumber(value: Double(code.rawValue))
  }
}

#endif
