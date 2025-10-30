package com.testlib

import android.os.Bundle
import com.facebook.react.ReactActivity
import com.facebook.react.ReactActivityDelegate

class TestlibWindowActivity : ReactActivity() {

  override fun getMainComponentName(): String {
    return intent?.getStringExtra(EXTRA_COMPONENT_NAME) ?: DEFAULT_COMPONENT_NAME
  }

  override fun createReactActivityDelegate(): ReactActivityDelegate {
    val componentName = getMainComponentName()
    return object : ReactActivityDelegate(this, componentName) {
      override fun getLaunchOptions(): Bundle? {
        val initialProps = Bundle()
        val windowId = intent?.getIntExtra(EXTRA_WINDOW_ID, -1) ?: -1
        if (windowId >= 0) {
          initialProps.putInt("windowId", windowId)
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
    const val DEFAULT_COMPONENT_NAME = "TestlibExample"
  }
}
