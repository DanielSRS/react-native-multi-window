package com.testlib

import android.os.Bundle
import com.facebook.react.ReactActivity
import com.facebook.react.ReactActivityDelegate

class TestlibWindowActivity : ReactActivity() {

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    val title = intent?.getStringExtra(EXTRA_WINDOW_TITLE)
    if (!title.isNullOrBlank()) {
      setTitle(title)
    }
  }

  override fun getMainComponentName(): String {
    return intent?.getStringExtra(EXTRA_COMPONENT_NAME) ?: DEFAULT_COMPONENT_NAME
  }

  override fun createReactActivityDelegate(): ReactActivityDelegate {
    val componentName = getMainComponentName()
    return object : ReactActivityDelegate(this, componentName) {
      override fun getLaunchOptions(): Bundle? {
        val initialProps = Bundle()
        val windowId = intent?.getIntExtra(EXTRA_WINDOW_ID, -1) ?: -1
        val windowOptions = intent?.getBundleExtra(EXTRA_WINDOW_OPTIONS)
        if (windowId >= 0) {
          initialProps.putInt("windowId", windowId)
        }
        if (windowOptions != null && !windowOptions.isEmpty) {
          initialProps.putBundle("windowOptions", windowOptions)
        }

        return if (initialProps.isEmpty) {
          null
        } else {
          initialProps
        }
      }
    }
  }

  companion object {
    const val EXTRA_COMPONENT_NAME = "com.testlib.extra.COMPONENT_NAME"
    const val EXTRA_WINDOW_ID = "com.testlib.extra.WINDOW_ID"
    const val EXTRA_WINDOW_OPTIONS = "com.testlib.extra.WINDOW_OPTIONS"
    const val EXTRA_WINDOW_TITLE = "com.testlib.extra.WINDOW_TITLE"
    const val DEFAULT_COMPONENT_NAME = "TestlibExample"
  }
}
