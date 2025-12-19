package com.multiwindow

import android.os.Bundle
import com.facebook.react.ReactActivity
import com.facebook.react.ReactActivityDelegate

class MultiWindowActivity : ReactActivity() {

  private var windowIdentifier: Long = INVALID_WINDOW_ID

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    windowIdentifier = intent?.getLongExtra(EXTRA_INSTANCE_ID, INVALID_WINDOW_ID) ?: INVALID_WINDOW_ID
    if (windowIdentifier > 0) {
      MultiWindowRegistry.register(windowIdentifier, this)
    }
  }

  override fun onDestroy() {
    if (windowIdentifier > 0) {
      MultiWindowRegistry.unregister(windowIdentifier, this)
    }
    super.onDestroy()
  }

  override fun createReactActivityDelegate(): ReactActivityDelegate = MultiWindowDelegate(this)

  private class MultiWindowDelegate(
    private val activity: MultiWindowActivity,
  ) : ReactActivityDelegate(activity, null) {

    override fun getMainComponentName(): String? =
      activity.intent?.getStringExtra(EXTRA_COMPONENT_NAME)?.takeUnless { it.isNullOrBlank() }

    override fun getLaunchOptions(): Bundle = Bundle().apply {
      val windowId = activity.intent?.getLongExtra(EXTRA_INSTANCE_ID, INVALID_WINDOW_ID) ?: INVALID_WINDOW_ID
      putDouble("windowId", windowId.toDouble())
      putBoolean("isAndroidMultiWindow", true)
    }
  }

  companion object {
    const val EXTRA_COMPONENT_NAME = "com.multiwindow.extra.COMPONENT_NAME"
    const val EXTRA_INSTANCE_ID = "com.multiwindow.extra.INSTANCE_ID"
    private const val INVALID_WINDOW_ID = -1L
  }
}
