package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.opengl.GLSurfaceView
import android.util.Base64
import android.util.Log
import java.nio.ByteBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class NativeEngine(private val context: Context) : GLSurfaceView.Renderer {
    private val nativeEngine = newNativeEngine(context.assets)

    companion object {
        init {
            System.loadLibrary("arshop")
        }
    }

    private external fun newNativeEngine(assetManager: AssetManager): Long
    private external fun onSurfaceCreated(pointer: Long)
    private external fun onSurfaceChanged(pointer: Long, rotation: Int, width: Int, height: Int)
    private external fun onDrawFrame(pointer: Long)
    private external fun onTouch(pointer: Long, x: Float, y: Float)
    private external fun onResume(pointer: Long, context: Context, activity: Activity)
    private external fun onPause(pointer: Long)
    private external fun takeFrame(pointer: Long): ByteBuffer

    private fun takeFrame(): ByteBuffer {
        return takeFrame(nativeEngine)
    }

    fun onTouch(x: Float, y: Float) {
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
        onSurfaceChanged(nativeEngine, context.resources.configuration.orientation - 1, width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        onDrawFrame(nativeEngine)

        takeFrame()
    }
}