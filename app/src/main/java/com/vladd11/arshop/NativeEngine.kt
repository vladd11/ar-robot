package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.content.res.AssetManager
import android.graphics.BitmapFactory
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.opengl.GLUtils
import android.util.Log
import java.io.File
import java.io.IOException
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class NativeEngine(private val context: Context) : GLSurfaceView.Renderer {
    private val filesDir = context.getExternalFilesDir("scripts")
    private val nativeEngine = newNativeEngine(filesDir.toString())

    init {
        assetManager = context.assets
        if (!File(filesDir, "socket.lua").exists()) {
            for (file in assetManager.list("libs")!!) {
                val outStream = File(filesDir, file).outputStream()
                val inStream = assetManager.open("libs/${file}")
                inStream.copyTo(outStream)
                outStream.close()
                inStream.close()
            }
        }
    }

    companion object {
        private lateinit var assetManager: AssetManager
        private const val TAG = "NativeEngine"

        init {
            System.loadLibrary("arshop")
        }

        @JvmStatic
        fun loadImage() {
            GLUtils.texImage2D(
                GLES20.GL_TEXTURE_2D, 0, try {
                    BitmapFactory.decodeStream(assetManager.open("trigrid.png"))
                } catch (e: IOException) {
                    Log.e(TAG, "Cannot open trigrid.png")
                    null
                }, 0
            )
        }
    }

    private external fun newNativeEngine(storagePath: String): Long
    private external fun onSurfaceCreated(pointer: Long)
    private external fun onSurfaceChanged(pointer: Long, rotation: Int, width: Int, height: Int)
    private external fun onDrawFrame(pointer: Long)
    private external fun onTouch(pointer: Long, x: Float, y: Float)
    private external fun onResume(pointer: Long, context: Context, activity: Activity)
    private external fun onPause(pointer: Long)

    fun onDestroy() {

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
        loadImage()
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