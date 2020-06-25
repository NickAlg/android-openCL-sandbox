package zt.mezon.graphomany.openclsandbox.ui.mactivity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.Toast;

import com.don11995.log.SimpleLog;
import com.google.android.material.bottomnavigation.BottomNavigationView;

import org.jetbrains.annotations.NotNull;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import zt.mezon.graphomany.openclsandbox.R;

public class MainActivity extends AppCompatActivity implements IMainViewNDKNavigator, IMainMVPView {
    private static final int CAMERA_PERMISSION_REQUEST = 1;
    private MainViewNDKHelper mNDKHelper;
    private IMainActivityPresenter mPresenter;
    private NavController mNavController;
    private AppBarConfiguration mAppBarConfiguration;
    private BottomNavigationView mBottomNavigationView;

    public NavController getNavController() {
        return mNavController;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
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
        mNDKHelper = new MainViewNDKHelper(this);
        mPresenter = new MainActivityPresenterImpl(this);
        mBottomNavigationView = findViewById(R.id.nav_view);
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        mAppBarConfiguration = new AppBarConfiguration.Builder(
                R.id.navigation_camera, R.id.navigation_dashboard, R.id.navigation_othercalc)
                .build();
        mNavController = Navigation.findNavController(this, R.id.nav_host_fragment);
        NavigationUI.setupActionBarWithNavController(this, mNavController, mAppBarConfiguration);
        NavigationUI.setupWithNavController(mBottomNavigationView, mNavController);
        mNDKHelper.initCL(0, 0);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NotNull String[] permissions,
                                           @NotNull int[] grantResults) {
        if (requestCode == CAMERA_PERMISSION_REQUEST) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            } else {
                String message = "Camera permission was not granted";
                SimpleLog.e(message);
                Toast.makeText(this, message, Toast.LENGTH_LONG).show();
            }
        } else {
            SimpleLog.e("Unexpected permission request");
        }
    }

    public MainViewNDKHelper getNDKHelper() {
        return mNDKHelper;
    }
}