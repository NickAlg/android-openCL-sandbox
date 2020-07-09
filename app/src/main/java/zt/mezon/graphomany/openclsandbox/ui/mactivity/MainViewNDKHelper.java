package zt.mezon.graphomany.openclsandbox.ui.mactivity;

import java.lang.ref.WeakReference;

public class MainViewNDKHelper {
    private final WeakReference<IMainViewNDKNavigator> mMainActivity;
    public MainViewNDKHelper(IMainViewNDKNavigator mainActivity) {
        mMainActivity = new WeakReference<IMainViewNDKNavigator>(mainActivity);
    }
    public native void coreFiltering(long mat_gray, int flag);

    public native void initCL(int width, int height);

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String takeClDataStringFromJNI();


    public native float[] takeTestClDataArrayFromJNI(float[] a, float[] b, float[] result) ;
}
