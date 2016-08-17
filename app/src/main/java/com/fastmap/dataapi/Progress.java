package com.fastmap.dataapi;

import android.util.Log;

public class Progress {
	private ProgressResult progressResult;
	private static Progress instance;

	public static Progress getInstance(ProgressResult progressResult) {
		if(instance!=null)
			instance = null;
		
		if (instance==null) {
			instance = new Progress(progressResult);
		}
		return instance;
	}

	private Progress() {
		super();
	}

	private Progress(ProgressResult progressResult) {
		super();
		this.progressResult = progressResult;
	}

	public double OnProgress(String sDescription, double nProgress) {
		Log.e(sDescription + " : ", Double.toString(nProgress));
		progressResult.onProgressResult(sDescription, nProgress);
		return nProgress;
	}
}
