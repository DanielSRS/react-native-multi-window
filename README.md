# react-native-multi-window

Open and manage extra windows / scenes in your react native app. Each window can host any React Native component you register and also shares memory/context between them, making comunication super easy. The module currently targets the following platforms:

- **Android**
- **iPadOS** (iPhone / classic iOS is not supported because the OS does not expose the necessary multi-window APIs)
- **macOS**
- **Windows**

> **Architecture support**
>
> - **Android / iOS / Windows:** This library is built as a TurboModule and therefore **requires React Native New Architecture** (Fabric + TurboModules) to function.
> - **macOS:** Both legacy and new architecture entry points are bundled because the RN macOS new architecture effort is still in flux. Only the legacy architecture is actively tested, so the new-arch path may be unstable.

## Installation

```sh
npm install react-native-multi-window
# or
yarn add react-native-multi-window

# iOS / macOS pods
# Only run `npx pod-install` if you manage the native iOS/macOS workspace yourself. Modern React Native tooling (Expo or the community CLI) generally runs CocoaPods for you, so there is no need to invoke it manually unless your workflow requires it.
npx pod-install
```

## iOS configuration

On iPad you must opt-in to multi-scene support so the library’s scene delegate can render your secondary windows. Add the following block to your **app target’s** `Info.plist` (for the example app this lives in `example/ios/MultiWindowExample/Info.plist`).

```xml
<key>UIApplicationSceneManifest</key>
<dict>
	<key>UIApplicationSupportsMultipleScenes</key>
	<true/>
	<key>UISceneConfigurations</key>
	<dict>
		<key>UIWindowSceneSessionRoleApplication</key>
		<array>
			<dict>
				<key>UISceneConfigurationName</key>
				<string>Default Configuration</string>
				<key>UISceneDelegateClassName</key>
				<string>MultiWindow.MWReactSceneDelegate</string>
			</dict>
		</array>
	</dict>
</dict>
```

That is the only manual change required—the library swizzles `RCTAppDelegate` for you and intercepts `RCTReactNativeFactory` to reuse the host bridge.

> iOS only allows additional scenes on iPad hardware. Calls on iPhone will resolve with an unsupported-platform error code.

## Other platforms

- **macOS:** no additional configuration required—just register the components you want to render.
- **Windows:** no additional configuration required—just register the components you want to render.
- **Android:** no additional configuration required—just register the components you want to render.

## Usage

Register the components you want to show in additional windows and call the exported helpers directly:

```tsx
// index.js / index.tsx
import { AppRegistry, Button, View } from 'react-native';
import { openNewWindow } from 'react-native-multi-window';

function Main() {
	return (
		<View>
			<Button
				title="Open dashboard window"
				onPress={async () => {
					const result = await openNewWindow({
						title: 'Dashboard',
						componentName: 'Dashboard',
						windows_WindowType: 0, // acrylic / mica only on Windows
					});

					console.log('openNewWindow resolved with:', result);
				}}
			/>
		</View>
	);
}

function Dashboard() {
	return <View>{/* render whatever you want */}</View>;
}

AppRegistry.registerComponent('Main', () => Main);
AppRegistry.registerComponent('Dashboard', () => Dashboard);
```

### API reference

`openNewWindow(options: WindowOptions): Promise<number>`

| Option | Type | Required | Description |
| --- | --- | --- | --- |
| `componentName` | `string` | ✅ | Registered component to render (matches `AppRegistry` name). |
| `title` | `string` | ✅ | Title shown on platforms that display window chrome / Scene title. |
| `windows_WindowType` | `0 \| 1 \| 2` | ⚠️ Windows only | Host style on Windows (`0` = standard, `1` = acrylic, `2` = mica). Ignored on iOS/macOS/Android. |

The promise always resolves with a numeric code:

- `> 0`: identifier of the window / scene that was opened (the actual meaning varies by platform: Android activity instance id, iOS/macOS scene/window id, Windows handle).
- `< 0`: **error**. Every platform uses a unique set of negative error codes (see below). Codes never overlap between platforms, so you can switch on the raw number to branch on platform-specific failures.

### Error codes by platform

#### Android
| Code | Meaning |
| --- | --- |
| `-71001` | No foreground activity available to launch `MultiWindowActivity`. |
| `-71002` | Invalid or empty `componentName`. |
| `-71003` | `startActivity` threw (likely due to missing multi-window permissions). |

#### iOS / iPadOS
| Code | Meaning |
| --- | --- |
| `-36991` | React Native bridge not yet available. |
| `-63912` | Invalid or empty `componentName`. |
| `-36853` | Embedding failed when composing the new scene. |
| `-63724` | Scene coordinator unavailable. |
| `-36755` | Platform does not support secondary scenes (e.g., iPhone). |
| `-63576` | `requestSceneSessionActivation` failed or the user dismissed the system sheet. |

#### macOS
| Code | Meaning |
| --- | --- |
| `-73001` | React Native bridge not yet available. |
| `-73002` | Invalid or empty `componentName`. |
| `-73003` | Failed to embed the React view in the new `NSWindow`. |
| `-73004` | Window manager unavailable (usually because the module was deallocated). |

#### Windows
| Code | Meaning |
| --- | --- |
| `-81273` | Failed to create a new `AppWindow`. |
| `-85674` | Could not retrieve the native window handle. |
| `-91234` | React Native host was missing. |
| `-23456` | Failed to create a `ReactViewHost`. |
| `-34567` | Unable to obtain a compositor for acrylic / mica. |
| `-45678` | Failed to get the container visual for composition. |
| `-56789` | Mica backdrop failed to initialize. |
| `-67890` | Acrylic backdrop failed to initialize. |

## Contributing

- [Development workflow](CONTRIBUTING.md#development-workflow)
- [Sending a pull request](CONTRIBUTING.md#sending-a-pull-request)
- [Code of conduct](CODE_OF_CONDUCT.md)

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
