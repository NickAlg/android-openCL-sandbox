package zt.mezon.graphomany.openclsandbox.ui.camera;

import android.app.Application;
import android.content.Context;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import zt.mezon.graphomany.openclsandbox.R;
import zt.mezon.graphomany.openclsandbox.datasource.ResourceProvider;

public class CameraHomeViewModel extends ViewModel {
    private Context mContext;
    private MutableLiveData<String> mText;


    public CameraHomeViewModel(Context context) {
        mContext = context;
        mText = new MutableLiveData<>();
        mText.setValue(context.getString(R.string.string_information));
    }

    public CameraHomeViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("Test");
    }

    public LiveData<String> getText() {
        return mText;
    }
}