package zt.mezon.graphomany.openclsandbox.ui.mactivity;

import java.lang.ref.WeakReference;

class MainActivityPresenterImpl implements IMainActivityPresenter {
    private final WeakReference<IMainMVPView> mMainActivity;
    public MainActivityPresenterImpl(IMainMVPView mainActivity) {
        mMainActivity = new WeakReference<IMainMVPView>(mainActivity);
    }
}
