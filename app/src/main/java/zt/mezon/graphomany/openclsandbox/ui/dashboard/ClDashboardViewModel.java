package zt.mezon.graphomany.openclsandbox.ui.dashboard;

import com.don11995.log.SimpleLog;

import java.lang.ref.WeakReference;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainViewNDKHelper;

public class ClDashboardViewModel extends ViewModel {

    private MutableLiveData<String> mText;
    private WeakReference<MainViewNDKHelper> mHelper;
    public ClDashboardViewModel( WeakReference<MainViewNDKHelper> helperWeakReferenceelper) {
        mHelper =helperWeakReferenceelper;
        mText = new MutableLiveData<>();
        String s = mHelper.get().stringFromJNI();
        SimpleLog.fd(s);
        mText.setValue(s);
    }

    public LiveData<String> getText() {
        return mText;
    }
}