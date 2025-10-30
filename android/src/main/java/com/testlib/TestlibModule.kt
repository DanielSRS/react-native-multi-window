package com.testlib

import android.content.Intent
import android.os.Build
import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
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

  override fun openNewWindow(promise: Promise) {
    val windowId = nextWindowId.incrementAndGet()
    val activity = currentActivity
    val context = reactApplicationContext

    val launchWindow = Runnable {
      try {
        val intent = Intent(context, TestlibWindowActivity::class.java).apply {
          putExtra(TestlibWindowActivity.EXTRA_WINDOW_ID, windowId)
          putExtra(TestlibWindowActivity.EXTRA_COMPONENT_NAME, DEFAULT_COMPONENT_NAME)
          addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
          addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
          if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT)
          }
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
    private val nextWindowId = AtomicInteger()
  }
}
