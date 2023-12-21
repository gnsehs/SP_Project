package com.example.trap

import android.os.Bundle
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.appcompat.app.AppCompatActivity

class CctvActivity : AppCompatActivity() {
    private lateinit var mWebView: WebView
    private lateinit var mWebSettings: WebSettings

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.cctv)

        mWebView = findViewById(R.id.webView)
        mWebView.webViewClient = WebViewClient()

        mWebSettings = mWebView.settings
        mWebSettings.javaScriptEnabled = true
        mWebSettings.setSupportZoom(false)
        mWebSettings.javaScriptCanOpenWindowsAutomatically = false
        mWebSettings.loadWithOverviewMode = true
        mWebSettings.useWideViewPort = true
        mWebSettings.setSupportZoom(false)
        mWebSettings.builtInZoomControls = false
        mWebSettings.layoutAlgorithm = WebSettings.LayoutAlgorithm.SINGLE_COLUMN
        mWebSettings.cacheMode = WebSettings.LOAD_NO_CACHE
        mWebSettings.domStorageEnabled = true

        mWebView.loadUrl("192.168.221.4:8000/")
    }
}
