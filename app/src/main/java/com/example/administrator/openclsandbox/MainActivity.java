package com.example.administrator.openclsandbox;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

import com.don11995.log.SimpleLog;

import org.jetbrains.annotations.NotNull;
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import androidx.core.app.ActivityCompat;

//

public class MainActivity extends Activity implements CvCameraViewListener2 {
    private static final int CAMERA_PERMISSION_REQUEST = 1;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private CameraBridgeViewBase mOpenCvCameraView;
    private Mat curFrame_gray;
    private Mat curFrame_rgba;
    private Mat prevFrame_gray;
    private Mat prevFrame_rgba;
    private Mat referenceFrame;
    private Button captureButton;
    private Button infoButton;
    private boolean viewFlag = false;
    private int mapFlag = 0;
    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            if (status == LoaderCallbackInterface.SUCCESS) {
                SimpleLog.i("OpenCV loaded successfully");

                // Load native library after(!) OpenCV initialization
                System.loadLibrary("native-lib");

                mOpenCvCameraView.enableView();
            } else {
                super.onManagerConnected(status);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        SimpleLog.i("called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Permissions for Android 6+
        ActivityCompat.requestPermissions(
                this,
                new String[]{Manifest.permission.CAMERA},
                CAMERA_PERMISSION_REQUEST
        );
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        SimpleLog.enableAllLogs();
        SimpleLog.setPrintReferences(true);

        mOpenCvCameraView = findViewById(R.id.main_surface);

        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);

        mOpenCvCameraView.setCvCameraViewListener(this);

        captureButton = findViewById(R.id.btn_takepicture);
        infoButton = findViewById(R.id.btn_info);

        captureButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                mapFlag = (mapFlag == 1) ? 0 : 1;
            }
        });

        infoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                viewFlag = false;
//                Intent intent = new Intent(MainActivity.this, AboutMe.class);
//                startActivity(intent);
            }
        });

    }

    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            SimpleLog.i("Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, this, mLoaderCallback);
        } else {
            SimpleLog.i("OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NotNull String[] permissions,
                                           @NotNull int[] grantResults) {
        if (requestCode == CAMERA_PERMISSION_REQUEST) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                //mOpenCvCameraView.setCameraPermissionGranted();
            } else {
                String message = "Camera permission was not granted";
                SimpleLog.e(message);
                Toast.makeText(this, message, Toast.LENGTH_LONG).show();
            }
        } else {
            SimpleLog.e("Unexpected permission request");
        }
    }

    //
    @Override
    public void onCameraViewStarted(int width, int height) {
        initCL();
        SimpleLog.i("OpenCL is initialized");

    }

    @Override
    public void onCameraViewStopped() {
    }

    //
    @Override
    public Mat onCameraFrame(CvCameraViewFrame frame) {
        // get current camera frame as OpenCV Mat object\


//        // native call to process current camera frame
        if (viewFlag) {
            // Mat mat_rgba = frame.rgba();
            Mat mat_gray = frame.rgba();
            Mat fMap, vMap;
            //curFrame_rgba = mat_rgba;
            curFrame_gray = mat_gray;
            coreFiltering(curFrame_gray.getNativeObjAddr(), mapFlag);
            SimpleLog.i("viewFlag: " + viewFlag);

            return curFrame_gray;
        } else {

            //Mat mat_rgba = frame.rgba();
            Mat mat_gray = frame.rgba();
            //curFrame_rgba = mat_rgba;
            curFrame_gray = mat_gray;
            //prevFrame_rgba = mat_rgba;
            //prevFrame_gray = mat_gray;
            SimpleLog.i("firstViewFlag: " + viewFlag);
            viewFlag = true;

            // return processed frame for live preview
            return curFrame_gray;
        }


    }

    private native void coreFiltering(long mat_gray, int flag);

    private native void initCL();

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
