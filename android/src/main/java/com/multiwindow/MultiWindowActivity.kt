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
      MultiWindowModule.emitWindowOpenedEvent(windowIdentifier, intent?.getStringExtra(EXTRA_TITLE))
    }
  }

  override fun onResume() {
    super.onResume()
    if (windowIdentifier > 0) {
      MultiWindowModule.emitWindowFocusEvent(windowIdentifier)
    }
  }

  override fun onDestroy() {
    if (windowIdentifier > 0) {
      MultiWindowRegistry.unregister(windowIdentifier, this)
      MultiWindowModule.emitWindowClosedEvent(windowIdentifier)
    }
    super.onDestroy()
  }

  override fun createReactActivityDelegate(): ReactActivityDelegate = MultiWindowDelegate(this)

  private class MultiWindowDelegate(
    private val activity: MultiWindowActivity,
  ) : ReactActivityDelegate(activity, null) {

    override fun getMainComponentName(): String? =
      activity.intent?.getStringExtra(EXTRA_COMPONENT_NAME)?.takeUnless { it.isNullOrBlank() }

    override fun getLaunchOptions(): Bundle {
      val windowId = activity.intent?.getLongExtra(EXTRA_INSTANCE_ID, INVALID_WINDOW_ID) ?: INVALID_WINDOW_ID
      val launchOptions = Bundle()

      activity.intent?.getBundleExtra(EXTRA_INITIAL_PROPS)?.let { launchOptions.putAll(it) }

      launchOptions.putDouble("windowId", windowId.toDouble())
      launchOptions.putBoolean("isAndroidMultiWindow", true)

      return launchOptions
    }
  }

  companion object {
    const val EXTRA_COMPONENT_NAME = "com.multiwindow.extra.COMPONENT_NAME"
    const val EXTRA_INSTANCE_ID = "com.multiwindow.extra.INSTANCE_ID"
    const val EXTRA_TITLE = "com.multiwindow.extra.TITLE"
    const val EXTRA_INITIAL_PROPS = "com.multiwindow.extra.INITIAL_PROPS"
    private const val INVALID_WINDOW_ID = -1L
  }
}
