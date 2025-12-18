package com.multiwindow

import android.content.Intent
import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.Promise
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.bridge.WritableMap
import com.facebook.react.module.annotations.ReactModule
import com.facebook.react.modules.core.DeviceEventManagerModule
import java.util.concurrent.atomic.AtomicLong

@ReactModule(name = MultiWindowModule.NAME)
class MultiWindowModule(
  private val reactContext: ReactApplicationContext,
) :
  NativeMultiWindowSpec(reactContext) {

  private val windowIdGenerator = AtomicLong(0)

  override fun getName(): String {
    return NAME
  }

  private fun emitLogEvent(
    context: ReactApplicationContext,
    payload: WritableMap,
  ) {
    context
      .getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter::class.java)
      .emit("MultiWindow/logs", payload)
  }

  @ReactMethod
  override fun openNewWindow(options: ReadableMap, promise: Promise) {
    emitLogEvent(
      reactContext,
      Arguments.createMap().apply {
        putString("function", "openNewWindow")
        putString("componentName", options.getStringOrNull("componentName") ?: "")
      },
    )

    val componentName = options.getStringOrNull("componentName")?.ifBlank { null }
    if (componentName == null) {
      promise.resolve(ErrorCodes.INVALID_COMPONENT_NAME.value)
      return
    }

    val activity = currentActivity
    if (activity == null) {
      promise.resolve(ErrorCodes.NO_FOREGROUND_ACTIVITY.value)
      return
    }

    val windowId = windowIdGenerator.incrementAndGet()

    val intent = Intent(activity, MultiWindowActivity::class.java).apply {
      addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
      putExtra(MultiWindowActivity.EXTRA_COMPONENT_NAME, componentName)
      putExtra(MultiWindowActivity.EXTRA_INSTANCE_ID, windowId)
    }

    runCatching {
      activity.startActivity(intent)
    }.onSuccess {
      promise.resolve(windowId.toDouble())
    }.onFailure {
      promise.resolve(ErrorCodes.START_ACTIVITY_FAILED.value)
    }
  }

  private fun ReadableMap.getStringOrNull(key: String): String? {
    return if (hasKey(key) && !isNull(key)) getString(key) else null
  }

  private enum class ErrorCodes(val value: Double) {
    NO_FOREGROUND_ACTIVITY(-71001.0),
    INVALID_COMPONENT_NAME(-71002.0),
    START_ACTIVITY_FAILED(-71003.0),
  }

  companion object {
    const val NAME = "MultiWindow"
  }
}
