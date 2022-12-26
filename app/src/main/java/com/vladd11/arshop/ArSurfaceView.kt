package com.vladd11.arshop

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.GestureDetector
import android.view.GestureDetector.SimpleOnGestureListener
import android.view.MotionEvent

@SuppressLint("ClickableViewAccessibility")
class ArSurfaceView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : GLSurfaceView(context, attrs) {
    val nativeEngine = NativeEngine(context)
    private var textListener: (str: String) -> Unit = {}

    fun setTextListener(textListener: (str: String) -> Unit) {
        this.textListener = textListener
    }

    init {
        nativeEngine.setTextListener { textListener(it) }

        val gestureDetector = GestureDetector(
            context,
            object : SimpleOnGestureListener() {
                override fun onSingleTapUp(e: MotionEvent): Boolean {
                    queueEvent {
                        nativeEngine.onTouch(e.x, e.y)
                    }
                    return true
                }

                override fun onDown(e: MotionEvent): Boolean {
                    return true
                }
            })

        setOnTouchListener { _, event -> gestureDetector.onTouchEvent(event) }

        preserveEGLContextOnPause = true
        setEGLContextClientVersion(2)
        setEGLConfigChooser(8, 8, 8, 8, 16, 0) // Alpha used for plane blending.
        setRenderer(nativeEngine)
        renderMode = RENDERMODE_CONTINUOUSLY
        setWillNotDraw(false)
    }


    fun resume(activity: Activity) {
        onResume()
        nativeEngine.onResume(activity)
    }

    fun destroy() {
        nativeEngine.onDestroy()
    }

    fun pause() {
        onPause()
        nativeEngine.onPause()
    }
}