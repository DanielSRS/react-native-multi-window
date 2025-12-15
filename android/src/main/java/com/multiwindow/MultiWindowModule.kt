package com.multiwindow

import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.WritableMap
import com.facebook.react.module.annotations.ReactModule
import com.facebook.react.modules.core.DeviceEventManagerModule

@ReactModule(name = MultiWindowModule.NAME)
class MultiWindowModule(
  private val reactContext: ReactApplicationContext,
) :
  NativeMultiWindowSpec(reactContext) {

  override fun getName(): String {
    return NAME
  }

  // Example method
  // See https://reactnative.dev/docs/native-modules-android
  override fun multiply(a: Double, b: Double): Double {
    val payload = createMultiplyPayload(a, b)
    emitLogEvent(reactContext, payload)
    return a * b
  }

  private fun createMultiplyPayload(a: Double, b: Double): WritableMap =
    Arguments.createMap().apply {
      putString("message", "MultiWindow multiply called with $a and $b")
      putDouble("a", a)
      putDouble("b", b)
    }

  private fun emitLogEvent(
    context: ReactApplicationContext,
    payload: WritableMap,
  ) {
    context
      .getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter::class.java)
      .emit("MultiWindow/logs", payload)
  }

  companion object {
    const val NAME = "MultiWindow"
  }
}
