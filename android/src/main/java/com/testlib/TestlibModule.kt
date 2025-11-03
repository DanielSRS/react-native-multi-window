package com.testlib

import android.content.Intent
import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import com.facebook.react.bridge.UiThreadUtil
import com.facebook.react.module.annotations.ReactModule
import java.util.concurrent.atomic.AtomicInteger

@ReactModule(name = TestlibModule.NAME)
class TestlibModule(reactContext: ReactApplicationContext) :
  NativeTestlibSpec(reactContext) {

  override fun getName(): String {
    return NAME
  }

  // Example method
  // See https://reactnative.dev/docs/native-modules-android
  override fun multiply(a: Double, b: Double): Double {
    return a * b
  }

  override fun openNewWindow(options: ReadableMap, promise: Promise) {
    val windowId = nextWindowId.incrementAndGet()
    val activity = currentActivity
    val context = reactApplicationContext
    val optionsBundle = Arguments.toBundle(options)
    val windowTitle = if (options.hasKey(KEY_TITLE) && !options.isNull(KEY_TITLE)) {
      options.getString(KEY_TITLE)
    } else {
      null
    }

    val launchWindow = Runnable {
      try {
        val intent = Intent(context, TestlibWindowActivity::class.java).apply {
          putExtra(TestlibWindowActivity.EXTRA_WINDOW_ID, windowId)
          putExtra(TestlibWindowActivity.EXTRA_COMPONENT_NAME, DEFAULT_COMPONENT_NAME)
          if (!windowTitle.isNullOrBlank()) {
            putExtra(TestlibWindowActivity.EXTRA_WINDOW_TITLE, windowTitle)
          }
          if (optionsBundle != null && !optionsBundle.isEmpty) {
            putExtra(TestlibWindowActivity.EXTRA_WINDOW_OPTIONS, optionsBundle)
          }
          addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
          addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
        }

        if (activity != null) {
          activity.startActivity(intent)
        } else {
          context.startActivity(intent)
        }

        promise.resolve(windowId.toDouble())
      } catch (throwable: Throwable) {
        promise.reject("E_OPEN_WINDOW", throwable)
      }
    }

    if (UiThreadUtil.isOnUiThread()) {
      launchWindow.run()
    } else {
      UiThreadUtil.runOnUiThread(launchWindow)
    }
  }

  companion object {
    const val NAME = "Testlib"
    private const val DEFAULT_COMPONENT_NAME = "TestlibExample"
    private const val KEY_TITLE = "title"
    private val nextWindowId = AtomicInteger()
  }
}
