package tn.securinets.ctf.net

import okhttp3.CertificatePinner
import okhttp3.OkHttpClient
import java.util.concurrent.TimeUnit

object NetworkConfig {

    const val HOST = "20.199.16.42"

    const val HTTP_PORT = 28000
    const val HTTPS_PORT = 28443
    const val ROGUE_SDK_PORT = 29090

    fun httpBaseUrl(): String = "http://$HOST:$HTTP_PORT"

    fun httpsBaseUrl(): String = "https://$HOST:$HTTPS_PORT"

    private const val CERT_PIN = "sha256/H2FIb+myuPsJTtKyui6C/vFQbgRmPZNb87mk7k4Yd8c="

    val standardClient: OkHttpClient = OkHttpClient.Builder()
        .connectTimeout(10, TimeUnit.SECONDS)
        .readTimeout(10, TimeUnit.SECONDS)
        .build()

    val pinnedClient: OkHttpClient = OkHttpClient.Builder()
        .connectTimeout(10, TimeUnit.SECONDS)
        .readTimeout(10, TimeUnit.SECONDS)
        .certificatePinner(
            CertificatePinner.Builder()
                .add(HOST, CERT_PIN)
                .build(),
        )
        .build()
}
