package com.vladd11.arshop

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class ArSurfaceView(context: Context, attrs: AttributeSet) : GLSurfaceView(context, attrs) {
    init {
        setEGLContextClientVersion(2)
        setRenderer(NativeRenderer())
        renderMode = RENDERMODE_WHEN_DIRTY
    }
}