package com.vladd11.arshop.jni

import java.nio.ByteBuffer

interface FrameCapturer {
    fun onImageCaptured(buffer: ByteBuffer, width: Int, height: Int)
}