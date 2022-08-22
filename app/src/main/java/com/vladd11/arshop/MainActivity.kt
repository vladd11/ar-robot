package com.vladd11.arshop

import android.opengl.GLSurfaceView
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.FragmentContainerView

class MainActivity : AppCompatActivity() {
    lateinit var glSurfaceView: GLSurfaceView;
    lateinit var fragmentView: FragmentContainerView;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        glSurfaceView = findViewById(R.id.surfaceView)
        fragmentView = findViewById(R.id.fragmentContainerView)
    }
}