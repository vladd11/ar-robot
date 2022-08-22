package com.vladd11.arshop

import android.app.NativeActivity
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.PersistableBundle
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    lateinit var glSurfaceView: GLSurfaceView;
    lateinit var fragmentView: GLSurfaceView;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        glSurfaceView = findViewById(R.id.surfaceView)
        fragmentView = findViewById(R.id.fragmentContainerView)
    }
}