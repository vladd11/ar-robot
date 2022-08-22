package com.vladd11.arshop

import android.content.Context
import android.opengl.GLSurfaceView

class ArSurfaceView(context: Context) : GLSurfaceView(context) {
    init {
        setEGLContextClientVersion(2)
        setRenderer(NativeRenderer())
        renderMode = RENDERMODE_WHEN_DIRTY
    }
}