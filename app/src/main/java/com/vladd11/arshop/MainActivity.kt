package com.vladd11.arshop

import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat

class MainActivity : AppCompatActivity() {
    private lateinit var glSurfaceView: ArSurfaceView

    companion object {
        const val REQUEST_CODE = 0
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        hideSystemBars()

        glSurfaceView = ArSurfaceView(this)
        setContentView(glSurfaceView)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String>, grantResults: IntArray
    ) {
        if (requestCode == REQUEST_CODE) {
            if(grantResults.isEmpty() || grantResults[0] == PackageManager.PERMISSION_DENIED) {
                Toast.makeText(this, "Please grant CAMERA permission", Toast.LENGTH_LONG).show()
                finish()
            }
        }

        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    override fun onResume() {
        super.onResume()

        if(checkSelfPermission(android.Manifest.permission.CAMERA) == PackageManager.PERMISSION_DENIED) {
            requestPermissions(arrayOf(android.Manifest.permission.CAMERA), 0)
            return
        }

        glSurfaceView.resume(this)
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.pause()
    }

    override fun onDestroy() {
        glSurfaceView.destroy()
        super.onDestroy()
    }

    private fun hideSystemBars() {
        WindowCompat.getInsetsController(window, window.decorView).apply {
            systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            hide(WindowInsetsCompat.Type.systemBars())
        }
    }
}