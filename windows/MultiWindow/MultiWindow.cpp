#include "pch.h"

#include <cmath>
#include <future>
#include <string>
#include "MultiWindow.h"
#include "Utilities.h"

namespace winrt::MultiWindow
{

namespace
{
constexpr double kInvalidCloseIdentifier = -71021.0;
constexpr double kWindowNotFound = -71022.0;
constexpr double kCloseRequestFailed = -71023.0;

bool NormalizeIdentifier(double id, uintptr_t& normalized) noexcept {
  if (!std::isfinite(id) || id <= 0) {
    return false;
  }

  const auto integral = static_cast<uintptr_t>(id);
  if (static_cast<double>(integral) != id) {
    return false;
  }

  normalized = integral;
  return true;
}
} // namespace

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void MultiWindow::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

void MultiWindow::RemoveWindow(winrt::Microsoft::UI::Windowing::AppWindow const& window) noexcept {
  const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(window.Id());
  const auto id = reinterpret_cast<uintptr_t>(hwnd);
  auto it = m_openWindows.find(id);
  if (it != m_openWindows.end()) {
    auto unloadAction = it->second.viewHost.UnloadViewInstance();
    if (it->second.type == WindowType::MICA && it->second.micaWindowData.has_value()) {
      auto& micaData = it->second.micaWindowData.value();
      micaData.compositionTarget.Close();
      micaData.controller.Close();
      micaData.rootVisual.Close();

      micaData.compositionTarget = nullptr;
      micaData.controller = nullptr;
      micaData.rootVisual = nullptr;
      it->second.micaWindowData.reset();
    }
    else if (it->second.type == WindowType::ACRYLIC && it->second.acrylicWindowData.has_value()) {
      auto& acrylicData = it->second.acrylicWindowData.value();
      if (acrylicData.configuration) {
        acrylicData.configuration = nullptr;
      }
      if (acrylicData.compositionTarget) {
        acrylicData.compositionTarget.Close();
        acrylicData.compositionTarget = nullptr;
      }
      if (acrylicData.controller) {
        acrylicData.controller.Close();
        acrylicData.controller = nullptr;
      }
      if (acrylicData.rootVisual) {
        acrylicData.rootVisual.Close();
        acrylicData.rootVisual = nullptr;
      }
      it->second.acrylicWindowData.reset();
    }
    m_openWindows.erase(id);
    EmitLogEvent(JSValueObject{
      {"function", "RemoveWindowCompleted"},
      {"remaining open windows", m_openWindows.size()},
      {"window id", id }
    });
    EmitWindowEvent(JSValueObject{
      {"type", 764},
      {"id", static_cast<double>(id)}
    });
    unloadAction.Completed([id](auto&&, auto&&) {
     });
  }
}

void MultiWindow::openNewWindow(WindowOptions&& options, ReactPromiseDouble&& result) noexcept {
  EmitLogEvent(JSValueObject{
    {"function", "openNewWindow"},
    {"title", options.title},
    {"componentName", options.componentName},
    {"number of open windows", m_openWindows.size()}
  });
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context, &openWindows = m_openWindows, this](
    WindowOptions opts,
    ReactPromiseDouble&& innerPromise) mutable {
      auto result = OpenReactWindow(
        context,
        opts,
        [this](winrt::Microsoft::UI::Windowing::AppWindow const& window) { RemoveWindow(window); },
        Utilities::EnsureThreadLocalCompositor
      );
      if (std::holds_alternative<ReactWindow>(result)) {
        auto r = std::get<ReactWindow>(result);
        const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(r.window.Id());
        const auto windowId = reinterpret_cast<uintptr_t>(hwnd);
        openWindows[windowId] = std::move(r);
        const auto windowIdDouble = static_cast<double>(windowId);
        innerPromise.Resolve(windowIdDouble);
        openWindows.at(windowId).window.Show();
        EmitWindowEvent(JSValueObject{
          {"type", 9873},
          {"id", windowIdDouble},
          {"title", opts.title}
        });
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

double MultiWindow::closeWindowBy(double id) noexcept {
  uintptr_t normalizedIdentifier = 0;
  const bool hasNormalizedIdentifier = NormalizeIdentifier(id, normalizedIdentifier);

  double result = kInvalidCloseIdentifier;
  std::string status = "invalid-input";
  int remainingWindows = static_cast<int>(m_openWindows.size());

  auto closeTask = [this,
                    hasNormalizedIdentifier,
                    &result,
                    &status,
                    &remainingWindows,
                    normalizedIdentifier]() noexcept {
    if (!hasNormalizedIdentifier) {
      remainingWindows = static_cast<int>(m_openWindows.size());
      return;
    }

    auto it = m_openWindows.find(normalizedIdentifier);
    if (it == m_openWindows.end()) {
      result = kWindowNotFound;
      status = "not-found";
      remainingWindows = static_cast<int>(m_openWindows.size());
      return;
    }

    auto window = it->second.window;
    bool requestSucceeded = false;

    if (window) {
      try {
        window.Destroy();
        requestSucceeded = true;
      }
      catch (...) {
        requestSucceeded = false;
      }
    }

    if (requestSucceeded) {
      result = static_cast<double>(normalizedIdentifier);
      status = "success";
    }
    else {
      result = kCloseRequestFailed;
      status = "request-failed";
    }

    remainingWindows = static_cast<int>(m_openWindows.size());
  };

  auto dispatcher = m_context.UIDispatcher();
  if (dispatcher && !dispatcher.HasThreadAccess()) {
    std::promise<void> completion;
    auto future = completion.get_future();

    dispatcher.Post([closeTask, completion = std::move(completion)]() mutable {
      closeTask();
      completion.set_value();
    });

    future.wait();
  }
  else {
    closeTask();
  }

  JSValueObject payload{
      {"function", "closeWindowBy"},
      {"requested id", id},
      {"status", status},
      {"remaining open windows", remainingWindows},
      {"result", result},
  };

  if (hasNormalizedIdentifier) {
    payload.insert_or_assign("normalized id", static_cast<double>(normalizedIdentifier));
  }

  EmitLogEvent(std::move(payload));

  return result;
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

void MultiWindow::EmitWindowEvent(JSValueObject payload) noexcept {
  if (!m_context) {
    return;
  }

  m_context.CallJSFunction(
      L"RCTDeviceEventEmitter",
      L"emit",
      [payload = std::move(payload)](React::IJSValueWriter const& writer) noexcept {
        React::WriteArgs(writer, "MultiWindow/event", payload);
      }
  );
}

} // namespace winrt::MultiWindow
