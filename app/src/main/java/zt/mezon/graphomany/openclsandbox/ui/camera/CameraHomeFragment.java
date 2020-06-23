package zt.mezon.graphomany.openclsandbox.ui.camera;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.don11995.log.SimpleLog;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import java.lang.ref.WeakReference;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import zt.mezon.graphomany.openclsandbox.R;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainActivity;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainViewNDKHelper;

public class CameraHomeFragment extends Fragment implements CameraBridgeViewBase.CvCameraViewListener2 {

    Mat mRgba;
    Mat mRgbaF;
    Mat mRgbaT;
    private CameraHomeViewModel mCameraHomeViewModel;
    private CameraBridgeViewBase mOpenCvCameraView;
    private Mat curFrame_gray;
    private Button captureButton;
    private Button infoButton;
    private boolean viewFlag = false;
    private int mapFlag = 0;
    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(getContext()) {
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
    private WeakReference<MainViewNDKHelper> mHelper;


    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
// If need no parameters
//        mCameraHomeViewModel =
//                new ViewModelProvider(this).get(CameraHomeViewModel.class);
//
        mCameraHomeViewModel =
                new ViewModelProvider(this, new ViewModelProvider.Factory() {
                    @NonNull
                    @Override
                    public <T extends ViewModel> T create(@NonNull Class<T> modelClass) {
                        return (T) new CameraHomeViewModel(getContext());
                    }
                }
                ).get(CameraHomeViewModel.class);


        View root = inflater.inflate(R.layout.fragment_clcam, container, false);
        infoButton = root.findViewById(R.id.btn_info);
        captureButton = root.findViewById(R.id.btn_takepicture);
        mOpenCvCameraView = root.findViewById(R.id.main_surface);
        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
        mOpenCvCameraView.setCvCameraViewListener(this);
        captureButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                mapFlag = (mapFlag == 1) ? 0 : 1;
            }
        });
        mOpenCvCameraView.setCameraPermissionGranted();
        mCameraHomeViewModel.getText().observe(getViewLifecycleOwner(), new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                infoButton.setText(s);
            }
        });
        return root;
    }

    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            SimpleLog.i("Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, getContext(), mLoaderCallback);
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
    public void onCameraViewStarted(int width, int height) {
        mHelper = new WeakReference<MainViewNDKHelper>(((MainActivity) getActivity()).getNDKHelper());

        SimpleLog.i("OpenCL is initialized");
        mRgba = new Mat(height, width, CvType.CV_8UC4);
        mRgbaF = new Mat(height, width, CvType.CV_8UC4);
        mRgbaT = new Mat(width, width, CvType.CV_8UC4);
    }

    @Override
    public void onCameraViewStopped() {

    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        // get current camera frame as OpenCV Mat object\
        mRgba = inputFrame.rgba();
//        return inputFrame.gray();
        // Rotate mRgba 90 degrees
//        Core.transpose(mRgba, mRgbaT);
//        Imgproc.resize(mRgbaT, mRgbaF, mRgbaF.size(), 0, 0, 0);
//        Core.flip(mRgba, mRgba, 1);

//        // native call to process current camera frame
        if (viewFlag) {
            // Mat mat_rgba = frame.rgba();
            Mat mat_gray = mRgba;
            Mat fMap, vMap;
            //curFrame_rgba = mat_rgba;
            curFrame_gray = mat_gray;
            mHelper.get().coreFiltering(curFrame_gray.getNativeObjAddr(), mapFlag);
            SimpleLog.i("viewFlag: " + viewFlag);

            return curFrame_gray;
        } else {

            curFrame_gray = mRgba;
            SimpleLog.i("firstViewFlag: " + viewFlag);
            viewFlag = true;

            // return processed frame for live preview
            return curFrame_gray;
        }

    }
}