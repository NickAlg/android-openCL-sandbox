package zt.mezon.graphomany.openclsandbox.ui.dashboard;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.don11995.log.SimpleLog;

import java.lang.ref.WeakReference;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProviders;
import zt.mezon.graphomany.openclsandbox.R;
import zt.mezon.graphomany.openclsandbox.ui.base.BaseFragment;
import zt.mezon.graphomany.openclsandbox.ui.camera.CameraHomeViewModel;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainViewNDKHelper;

public class ClDashboardFragment extends BaseFragment {

    private ClDashboardViewModel mClDashboardViewModel;
    private WeakReference<MainViewNDKHelper> mHelper;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        mHelper = new WeakReference<MainViewNDKHelper>(getAppActivity().getNDKHelper());
        mClDashboardViewModel =
                new ViewModelProvider(this, new ViewModelProvider.Factory() {
                    @NonNull
                    @Override
                    public <T extends ViewModel> T create(@NonNull Class<T> modelClass) {
                        return (T) new ClDashboardViewModel( mHelper);
                    }
                }
                ).get(ClDashboardViewModel.class);


        View root = inflater.inflate(R.layout.fragment_dashboard, container, false);
        final TextView textView = root.findViewById(R.id.text_dashboard);
        mClDashboardViewModel.getText().observe(getViewLifecycleOwner(), new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                textView.setText(s);
            }
        });

        return root;
    }
}