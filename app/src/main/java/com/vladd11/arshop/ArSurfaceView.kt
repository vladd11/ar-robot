package com.vladd11.arshop

import android.app.Activity
import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class ArSurfaceView(context: Context, attrs: AttributeSet) : GLSurfaceView(context, attrs) {
    private val nativeEngine = NativeEngine(context)

    init {
        preserveEGLContextOnPause = true;
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        setRenderer(nativeEngine);
        renderMode = RENDERMODE_CONTINUOUSLY;
        setWillNotDraw(false);
    }

    fun resume(activity: Activity) {
        nativeEngine.onResume(activity)
    }
}