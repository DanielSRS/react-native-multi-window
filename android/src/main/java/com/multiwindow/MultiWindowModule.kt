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
import java.lang.ref.WeakReference
import java.util.concurrent.atomic.AtomicLong

@ReactModule(name = MultiWindowModule.NAME)
class MultiWindowModule(
  private val reactContext: ReactApplicationContext,
) :
  NativeMultiWindowSpec(reactContext) {

  init {
    registerModuleContext(reactContext)
  }

  override fun initialize() {
    super.initialize()
    registerModuleContext(reactContext)
  }

  override fun invalidate() {
    unregisterModuleContext(reactContext)
    super.invalidate()
  }

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

  private fun registerModuleContext(context: ReactApplicationContext) {
    Companion.registerModuleContext(context)
  }

  private fun unregisterModuleContext(context: ReactApplicationContext) {
    Companion.unregisterModuleContext(context)
  }

  @ReactMethod
  override fun openNewWindow(options: ReadableMap, promise: Promise) {
    val componentName = options.getStringOrNull("componentName")?.ifBlank { null }
    val title = options.getStringOrNull("title") ?: ""

    emitLogEvent(
      reactContext,
      Arguments.createMap().apply {
        putString("function", "openNewWindow")
        putString("componentName", componentName ?: "")
        putString("title", title)
      },
    )

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

    val initialProps = options.getMapOrNull("initialProps")?.let { Arguments.toBundle(it) }

    val intent = Intent(activity, MultiWindowActivity::class.java).apply {
      addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
      putExtra(MultiWindowActivity.EXTRA_COMPONENT_NAME, componentName)
      putExtra(MultiWindowActivity.EXTRA_INSTANCE_ID, windowId)
      putExtra(MultiWindowActivity.EXTRA_TITLE, title)
      initialProps?.let {
        putExtra(MultiWindowActivity.EXTRA_INITIAL_PROPS, it)
      }
    }

    runCatching {
      activity.startActivity(intent)
    }.onSuccess {
      promise.resolve(windowId.toDouble())
    }.onFailure {
      promise.resolve(ErrorCodes.START_ACTIVITY_FAILED.value)
    }
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  override fun closeWindowBy(id: Double): Double {
    val normalizedIdentifier = normalizeIdentifier(id)
    var result = ErrorCodes.INVALID_CLOSE_IDENTIFIER.value
    var status = "invalid-input"

    if (normalizedIdentifier != null) {
      when (MultiWindowRegistry.requestClose(normalizedIdentifier)) {
        MultiWindowRegistry.CloseResult.SUCCESS -> {
          result = normalizedIdentifier.toDouble()
          status = "success"
        }
        MultiWindowRegistry.CloseResult.NOT_FOUND -> {
          result = ErrorCodes.WINDOW_NOT_FOUND.value
          status = "not-found"
        }
        MultiWindowRegistry.CloseResult.FAILED -> {
          result = ErrorCodes.CLOSE_REQUEST_FAILED.value
          status = "request-failed"
        }
      }
    }

    emitLogEvent(
      reactContext,
      Arguments.createMap().apply {
        putString("function", "closeWindowBy")
        putDouble("requested id", id)
        normalizedIdentifier?.let { putDouble("normalized id", it.toDouble()) }
        putString("status", status)
        putInt("remaining open windows", MultiWindowRegistry.activeWindowCount())
        putDouble("result", result)
      },
    )

    return result
  }

  private fun ReadableMap.getStringOrNull(key: String): String? {
    return if (hasKey(key) && !isNull(key)) getString(key) else null
  }

  private fun ReadableMap.getMapOrNull(key: String): ReadableMap? {
    return if (hasKey(key) && !isNull(key)) getMap(key) else null
  }

  private fun normalizeIdentifier(id: Double): Long? {
    if (!id.isFinite()) {
      return null
    }

    if (id <= 0) {
      return null
    }

    val longValue = id.toLong()
    return if (longValue.toDouble() == id) longValue else null
  }

  private enum class ErrorCodes(val value: Double) {
    NO_FOREGROUND_ACTIVITY(-71001.0),
    INVALID_COMPONENT_NAME(-71002.0),
    START_ACTIVITY_FAILED(-71003.0),
    INVALID_CLOSE_IDENTIFIER(-71021.0),
    WINDOW_NOT_FOUND(-71022.0),
    CLOSE_REQUEST_FAILED(-71023.0),
  }

  companion object {
    const val NAME = "MultiWindow"

    @Volatile
    private var reactContextRef: WeakReference<ReactApplicationContext>? = null

    private fun emitWindowEvent(
      context: ReactApplicationContext,
      payload: WritableMap,
    ) {
      context
        .getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter::class.java)
        .emit("MultiWindow/event", payload)
    }

    internal fun emitWindowClosedEvent(id: Long) {
      val context = reactContextRef?.get() ?: return

      emitWindowEvent(
        context,
        Arguments.createMap().apply {
          putInt("type", 764)
          putDouble("id", id.toDouble())
        },
      )
    }

    internal fun emitWindowOpenedEvent(id: Long, title: String?) {
      val context = reactContextRef?.get() ?: return

      emitWindowEvent(
        context,
        Arguments.createMap().apply {
          putInt("type", 9873)
          putDouble("id", id.toDouble())
          putString("title", title ?: "")
        },
      )
    }

    internal fun registerModuleContext(context: ReactApplicationContext) {
      reactContextRef = WeakReference(context)
    }

    internal fun unregisterModuleContext(context: ReactApplicationContext) {
      val stored = reactContextRef?.get()
      if (stored == null || stored == context) {
        reactContextRef = null
      }
    }
  }
}
