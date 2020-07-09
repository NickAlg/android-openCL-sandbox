package zt.mezon.graphomany.openclsandbox.ui.testcalc;

import com.don11995.log.SimpleLog;

import java.lang.ref.WeakReference;
import java.util.Arrays;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainViewNDKHelper;

public class TestCalcViewModel extends ViewModel {

    private static final int ARRAY_SIZE = 20;
    float[] mResult = new float[ARRAY_SIZE];
    float[] mTesResult = new float[ARRAY_SIZE];
    float[] mA = new float[ARRAY_SIZE];
    float[] mB = new float[ARRAY_SIZE];
    private MutableLiveData<String> mText;
    private WeakReference<MainViewNDKHelper> mHelper;


    public TestCalcViewModel(WeakReference<MainViewNDKHelper> helperWeakReferenceelper) {
        mHelper = helperWeakReferenceelper;
        mText = new MutableLiveData<>();
        startfill(mA, mB);
        mResult = mHelper.get().takeTestClDataArrayFromJNI(mA, mB, mResult);
        String s = "In A =[" +
                Arrays.toString(mA) +
                "] \n" +
                "In B =[" +
                Arrays.toString(mB) +
                "] \n" +
                "Test Out Result =[" +
                Arrays.toString(mTesResult) +
                "] \n"+
                "Out Result =[" +
                Arrays.toString(mResult) +
                "] \n";
        SimpleLog.d(s);
        mText.setValue(s);
    }

    private void startfill(float[] a, float[] b) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] = (float) i;
            b[i] = (float) (ARRAY_SIZE - i);
        }
        for (int j = 0; j < ARRAY_SIZE; j++) {
            mTesResult[j] = a[j] + b[j];
        }
    }

    public LiveData<String> getText() {
        return mText;
    }
}

