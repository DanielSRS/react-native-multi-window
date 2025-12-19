package com.multiwindow

import android.os.Handler
import android.os.Looper
import java.lang.ref.WeakReference
import java.util.concurrent.ConcurrentHashMap

internal object MultiWindowRegistry {
  enum class CloseResult {
    SUCCESS,
    NOT_FOUND,
    FAILED,
  }

  private val handler = Handler(Looper.getMainLooper())
  private val activities = ConcurrentHashMap<Long, WeakReference<MultiWindowActivity>>()

  fun register(id: Long, activity: MultiWindowActivity) {
    activities[id] = WeakReference(activity)
  }

  fun unregister(id: Long, activity: MultiWindowActivity?) {
    val tracked = activities[id]?.get()
    if (tracked == null || tracked === activity) {
      activities.remove(id)
    }
  }

  fun requestClose(id: Long): CloseResult {
    val trackedActivity = activities[id]?.get()
    if (trackedActivity == null) {
      activities.remove(id)
      return CloseResult.NOT_FOUND
    }

    if (trackedActivity.isFinishing || trackedActivity.isDestroyed) {
      activities.remove(id)
      return CloseResult.NOT_FOUND
    }

    val finishRunnable = Runnable {
      trackedActivity.finishAndRemoveTask()
    }

    return runCatching {
      if (Looper.myLooper() == Looper.getMainLooper()) {
        finishRunnable.run()
      } else {
        handler.post(finishRunnable)
      }
    }.fold(
      onSuccess = { CloseResult.SUCCESS },
      onFailure = {
        CloseResult.FAILED
      },
    )
  }

  fun activeWindowCount(): Int {
    val iterator = activities.entries.iterator()
    var count = 0
    while (iterator.hasNext()) {
      val entry = iterator.next()
      if (entry.value.get() == null) {
        iterator.remove()
      } else {
        count += 1
      }
    }
    return count
  }
}
