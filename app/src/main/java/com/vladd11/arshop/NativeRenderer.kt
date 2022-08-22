package com.vladd11.arshop

import android.opengl.GLES20
import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class NativeRenderer : GLSurfaceView.Renderer {
    private val nativeRenderer: Long;

    companion object {
        init {
            System.loadLibrary("arshop");
        }
    }

    init {
        nativeRenderer = newNativeEngine();
    }

    private external fun newNativeEngine(): Long;
    private external fun onSurfaceCreated(pointer: Long);
    private external fun onDrawFrame(pointer: Long);

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        onSurfaceCreated(nativeRenderer);
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        onDrawFrame(nativeRenderer);
    }
}