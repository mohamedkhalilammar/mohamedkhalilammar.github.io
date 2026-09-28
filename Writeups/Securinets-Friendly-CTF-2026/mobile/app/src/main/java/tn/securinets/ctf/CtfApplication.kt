package tn.securinets.ctf

import android.app.Application
import com.metricflow.sdk.MetricFlowKit

class CtfApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        MetricFlowKit.track(this, "app_session_start")
    }
}
