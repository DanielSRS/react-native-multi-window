#include "pch.h"

#include <sstream>

#include "MultiWindow.h"

namespace winrt::MultiWindow
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void MultiWindow::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

double MultiWindow::multiply(double a, double b) noexcept {
  EmitLogEvent(CreateMultiplyPayload(a, b));
  return a * b;
}

void MultiWindow::openNewWindow(WindowOptions&& options, ReactPromiseDouble&& result) noexcept {
  EmitLogEvent(winrt::Microsoft::ReactNative::JSValueObject{
    {"function", "openNewWindow"},
    {"title", options.title},
    {"componentName", options.componentName},
  });
  result.Resolve(123.0);
}

winrt::Microsoft::ReactNative::JSValueObject MultiWindow::CreateMultiplyPayload(double a, double b) noexcept {
  std::ostringstream messageStream;
  messageStream << "MultiWindow multiply called with " << a << " and " << b;

  return winrt::Microsoft::ReactNative::JSValueObject{
      {"message", messageStream.str()},
      {"a", a},
      {"b", b},
  };
}

void MultiWindow::EmitLogEvent(winrt::Microsoft::ReactNative::JSValueObject payload) noexcept {
  if (!m_context) {
    return;
  }

  m_context.CallJSFunction(
      L"RCTDeviceEventEmitter",
      L"emit",
      [payload = std::move(payload)](React::IJSValueWriter const& writer) noexcept {
        React::WriteArgs(writer, "MultiWindow/logs", payload);
      }
  );
}

} // namespace winrt::MultiWindow
