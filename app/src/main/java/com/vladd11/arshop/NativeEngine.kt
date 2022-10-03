package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.opengl.GLSurfaceView
import android.os.Handler
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class NativeEngine(private val context: Context) : GLSurfaceView.Renderer {
    private val nativeEngine = newNativeEngine()
    private lateinit var handler: Handler

    companion object {
        private const val TAG = "NativeEngine"
        private const val TOUCH_MESSAGE = 0xD1A1ED

        init {
            System.loadLibrary("arshop")
        }
    }

    private external fun newNativeEngine(): Long
    private external fun onSurfaceCreated(pointer: Long)
    private external fun onSurfaceChanged(pointer: Long, rotation: Int, width: Int, height: Int)
    private external fun onDrawFrame(pointer: Long)
    private external fun onTouch(pointer: Long, x: Float, y: Float)
    private external fun onResume(pointer: Long, context: Context, activity: Activity)
    private external fun onPause(pointer: Long)

    fun onDestroy() {

    }

    fun onTouch(x: Float, y: Float) {
        handler.sendEmptyMessage(TOUCH_MESSAGE)
        onTouch(nativeEngine, x, y)
    }

    fun onResume(activity: Activity) {
        onResume(nativeEngine, activity.applicationContext, activity)
    }

    fun onPause() {
        onPause(nativeEngine)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        onSurfaceCreated(nativeEngine)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        onSurfaceChanged(
            nativeEngine,
            context.resources.configuration.orientation - 1,
            width,
            height
        )
    }

    override fun onDrawFrame(gl: GL10?) {
        onDrawFrame(nativeEngine)
    }
}