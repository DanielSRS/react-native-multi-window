package com.multiwindow

import android.os.Bundle
import com.facebook.react.ReactActivity
import com.facebook.react.ReactActivityDelegate

class MultiWindowActivity : ReactActivity() {

  override fun createReactActivityDelegate(): ReactActivityDelegate = MultiWindowDelegate(this)

  private class MultiWindowDelegate(
    private val activity: MultiWindowActivity,
  ) : ReactActivityDelegate(activity, null) {

    override fun getMainComponentName(): String? =
      activity.intent?.getStringExtra(EXTRA_COMPONENT_NAME)?.takeUnless { it.isNullOrBlank() }

    override fun getLaunchOptions(): Bundle = Bundle().apply {
      val windowId = activity.intent?.getLongExtra(EXTRA_INSTANCE_ID, -1L) ?: -1L
      putDouble("windowId", windowId.toDouble())
      putBoolean("isAndroidMultiWindow", true)
    }
  }

  companion object {
    const val EXTRA_COMPONENT_NAME = "com.multiwindow.extra.COMPONENT_NAME"
    const val EXTRA_INSTANCE_ID = "com.multiwindow.extra.INSTANCE_ID"
  }
}
