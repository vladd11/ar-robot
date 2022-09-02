package com.vladd11.arshop

import android.content.Context
import android.graphics.Bitmap
import com.google.android.gms.tasks.Tasks
import com.google.android.gms.tasks.Tasks.await
import com.google.android.gms.tflite.client.TfLiteInitializationOptions
import com.google.android.gms.tflite.java.TfLite
import org.tensorflow.lite.support.image.TensorImage
import org.tensorflow.lite.task.core.BaseOptions
import org.tensorflow.lite.task.gms.vision.TfLiteVision
import org.tensorflow.lite.task.gms.vision.detector.ObjectDetector


class PriceTagDetector(context: Context) {
    private val objectDetector: ObjectDetector
    private var useGPU: Boolean = true

    init {
        await(TfLiteVision.initialize(
            context,
            TfLiteInitializationOptions.builder().setEnableGpuDelegateSupport(true).build()
        ).continueWithTask {
            if (it.exception != null) {
                useGPU = false;
                return@continueWithTask TfLite.initialize(context)
            } else {
                return@continueWithTask Tasks.forResult<Void>(null)
            }
        })

        val options =
            if (useGPU) {
                ObjectDetector.ObjectDetectorOptions.builder()
                    .setBaseOptions(BaseOptions.builder().useGpu().build())
                    .setMaxResults(1)
                    .build()
            } else {
                ObjectDetector.ObjectDetectorOptions.builder()
                    .setMaxResults(1)
                    .build()
            }

        objectDetector = ObjectDetector.createFromFileAndOptions(context, "modelFile", options)
    }

    fun detect(bitmap: Bitmap) {
        objectDetector.detect(TensorImage.fromBitmap(bitmap))
    }
}