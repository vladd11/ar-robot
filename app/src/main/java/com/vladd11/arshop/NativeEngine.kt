package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.graphics.Bitmap
import android.opengl.GLSurfaceView
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.Log
import androidx.annotation.Keep
import com.vladd11.arshop.jni.FrameCapturer
import java.nio.ByteBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.concurrent.thread


class NativeEngine(private val context: Context) : GLSurfaceView.Renderer, FrameCapturer {
    private val nativeEngine = newNativeEngine()
    private lateinit var priceTagDetector: PriceTagDetector
    private lateinit var handler: Handler

    init {
        thread(isDaemon = true) {
            Looper.prepare()
            handler = Handler(Looper.getMainLooper(), Handler.Callback {
                if (it.what == TOUCH_MESSAGE) {
                    takeFrame()
                    return@Callback true
                }
                return@Callback false
            })

            priceTagDetector = PriceTagDetector(context)
        }
    }

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
    private external fun takeFrame(pointer: Long)

    private fun takeFrame() {
        takeFrame(nativeEngine)
    }

    @Keep
    override fun onImageCaptured(buffer: ByteBuffer?, width: Int, height: Int) {
        if (buffer == null) {
            Log.d(TAG, "Buffer is null")
            return
        }

        val bitmap =
            Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
                copyPixelsFromBuffer(buffer)
            }
        try {
            priceTagDetector.detect(bitmap)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    fun onDestroy() {
        priceTagDetector.close()
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