package com.vladd11.arshop

import android.content.Context
import org.tensorflow.lite.task.gms.vision.TfLiteVision
import org.tensorflow.lite.InterpreterApi
import org.tensorflow.lite.InterpreterApi.Options.TfLiteRuntime

class PriceTagDetector(context: Context) {
    init {
        TfLiteVision.initialize(context)
        val tflite by lazy {
            Interpreter(
                FileUtil.loadMappedFile(this, MODEL_PATH),
                Interpreter.Options().addDelegate(nnApiDelegate))
        }
    }

    fun detect() {

    }
}