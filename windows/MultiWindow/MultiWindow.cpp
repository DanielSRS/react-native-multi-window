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
  EmitLogEvent(JSValueObject{
    {"function", "openNewWindow"},
    {"title", options.title},
    {"componentName", options.componentName},
    {"number of open windows", m_openWindows.size()}
  });
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context, &openWindows = m_openWindows](
    WindowOptions opts,
    ReactPromiseDouble&& innerPromise) mutable {
      auto result = OpenReactWindow(context, opts);
      if (std::holds_alternative<ReactWindow>(result)) {
        auto r = std::get<ReactWindow>(result);
        const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(r.window.Id());
        openWindows[reinterpret_cast<uintptr_t>(hwnd)] = std::move(r);
        innerPromise.Resolve((double) reinterpret_cast<uintptr_t>(hwnd));
        openWindows.at(reinterpret_cast<uintptr_t>(hwnd)).window.Show();
      }
      else {
        auto errorCode = std::get<ReactWindowCreationError>(result);
        innerPromise.Resolve((double) errorCode);
      }
    };

  if (dispatcher && dispatcher.HasThreadAccess()) {
    fulfill(std::move(options), std::move(result));
    return;
  }

  if (dispatcher) {
    dispatcher.Post([fulfill,
      opts = std::move(options),
      promise = std::move(result)]() mutable {
        fulfill(std::move(opts), std::move(promise));
      });
    return;
  }
  result.Resolve(123.0);
}

JSValueObject MultiWindow::CreateMultiplyPayload(double a, double b) noexcept {
  std::ostringstream messageStream;
  messageStream << "MultiWindow multiply called with " << a << " and " << b;

  return JSValueObject{
      {"message", messageStream.str()},
      {"a", a},
      {"b", b},
  };
}

void MultiWindow::EmitLogEvent(JSValueObject payload) noexcept {
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
