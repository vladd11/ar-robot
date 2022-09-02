package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.graphics.Bitmap
import android.opengl.GLSurfaceView
import com.vladd11.arshop.jni.FrameCapturer
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class NativeEngine(private val context: Context) : GLSurfaceView.Renderer, FrameCapturer {
    private val nativeEngine = newNativeEngine()
    //private val priceTagDetector = PriceTagDetector(context)
    private var frameWidth: Int = -1
    private var frameHeight: Int = -1

    companion object {
        @Suppress("unused")
        private const val TAG = "NativeEngine"

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
    private external fun takeFrame(pointer: Long): ByteBuffer?

    private fun takeFrame(): ByteBuffer? {
        return takeFrame(nativeEngine)
    }

    fun onTouch(x: Float, y: Float) {
        onTouch(nativeEngine, x, y)

        val buffer = takeFrame() // Usually takes around 30ms
        if (buffer != null) {
            val bitmap =
                Bitmap.createBitmap(frameWidth, frameHeight, Bitmap.Config.ARGB_8888).apply {
                    copyPixelsFromBuffer(buffer)
                }
            try {
                val output = FileOutputStream(
                    File(
                        context.cacheDir.toString(),
                        "test.jpg"
                    )
                )
                //priceTagDetector.detect(bitmap)

                bitmap.compress(Bitmap.CompressFormat.JPEG, 100, output)
                output.flush()
                output.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
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

    override fun onFrameSizeChanged(width: Int, height: Int) {
        frameWidth = width
        frameHeight = height
    }
}