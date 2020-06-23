package zt.mezon.graphomany.openclsandbox;

import com.don11995.log.SimpleLog;

import androidx.multidex.MultiDexApplication;
import zt.mezon.graphomany.openclsandbox.datasource.ResourceProvider;

public class App extends MultiDexApplication {
    static {
        System.loadLibrary("native-lib");
    }


    @Override
    public void onCreate() {
        super.onCreate();
        SimpleLog.enableAllLogs();
        SimpleLog.setPrintReferences(true);
    }
}
