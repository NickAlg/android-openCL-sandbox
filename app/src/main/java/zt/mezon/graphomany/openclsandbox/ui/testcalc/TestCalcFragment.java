package zt.mezon.graphomany.openclsandbox.ui.testcalc;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.lang.ref.WeakReference;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProviders;
import zt.mezon.graphomany.openclsandbox.R;
import zt.mezon.graphomany.openclsandbox.ui.base.BaseFragment;
import zt.mezon.graphomany.openclsandbox.ui.dashboard.ClDashboardViewModel;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainViewNDKHelper;

public class TestCalcFragment extends BaseFragment {

    private TestCalcViewModel mTestCalcViewModel;
    private WeakReference<MainViewNDKHelper> mHelper;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        mHelper = new WeakReference<MainViewNDKHelper>(getAppActivity().getNDKHelper());
        mTestCalcViewModel =
                new ViewModelProvider(this, new ViewModelProvider.Factory() {
                    @NonNull
                    @Override
                    public <T extends ViewModel> T create(@NonNull Class<T> modelClass) {
                        return (T) new TestCalcViewModel( mHelper);
                    }
                }
                ).get(TestCalcViewModel.class);



        View root = inflater.inflate(R.layout.fragment_othercalc, container, false);
        final TextView textView = root.findViewById(R.id.text_notifications);
        mTestCalcViewModel.getText().observe(getViewLifecycleOwner(), new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                textView.setText(s);
            }
        });
        return root;
    }
}