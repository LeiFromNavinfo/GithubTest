package com.fastmap.dataapi;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    private static String TAG = "Editor";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        DataNativeApi.SetRootPath("/storage/sdcard0/fastmap3");

        DataNativeApi.SetUserId("4396");

        Log.e(TAG, "get path");

        String ss = DataNativeApi.GetDataPath();

        Log.e(TAG, ss);

	Log.e(TAG2, ss);
    }

    static
    {
        try
        {
            System.loadLibrary("curl");
            System.loadLibrary("geos");
            System.loadLibrary("iconv");
            System.loadLibrary("lzma");
            System.loadLibrary("proj");
            System.loadLibrary("xml2");
            System.loadLibrary("sqlite_nds");
            System.loadLibrary("spatialite");
            System.loadLibrary("Editor");
        }
        catch (UnsatisfiedLinkError e)
        {
            // TODO: handle exception
            Log.e(TAG, "Couldn't load native libs");
            e.printStackTrace();
        }

    }
}
