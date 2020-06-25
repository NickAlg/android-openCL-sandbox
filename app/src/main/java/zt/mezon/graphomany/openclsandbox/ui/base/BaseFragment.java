package zt.mezon.graphomany.openclsandbox.ui.base;

import android.content.Context;

import com.don11995.log.SimpleLog;

import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import zt.mezon.graphomany.openclsandbox.ui.mactivity.MainActivity;

/**
 * Created by NickZT on 24.06.2020.
 */
public class BaseFragment extends Fragment {
    private boolean mIsAttached = false;

    public MainActivity getAppActivity() {
        MainActivity mainActivity = (MainActivity) getActivity();
        if (mainActivity == null) {
            throw new NullPointerException();
        } else {
            return (MainActivity) getActivity();
        }
    }

    public BaseFragment() {
        // Required empty public constructor
    }

    public NavController getNavigator() {
        return getAppActivity().getNavController();
    }

    public void navigateBack() {
        getAppActivity().getNavController().popBackStack();
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mIsAttached = true;
        SimpleLog.fd("---" + this.getId() + "-- %s has been attached", this.getClass().getSimpleName());
    }

    @Override
    public void onDetach() {
        mIsAttached = false;
        super.onDetach();
    }

    public boolean isAttached() {
        return mIsAttached;
    }
}
