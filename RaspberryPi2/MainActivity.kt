package com.example.trap

import android.content.AsyncQueryHandler
import android.location.Geocoder
import android.location.Address
import android.util.Log
import android.net.Uri
import android.os.AsyncTask
import android.os.Bundle
import android.content.Intent
import android.widget.EditText
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.io.*
import java.net.*

class MainActivity : AppCompatActivity() {

    val ip = "192.168.221.4"
    var port : Int = 8080


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        var portedit = findViewById<EditText>(R.id.editport)

        val portButton: Button = findViewById(R.id.portbutton)
        val ledButton: Button = findViewById(R.id.ledbutton)
        val doorButton: Button = findViewById(R.id.doorbutton)
        val gpsButton: Button = findViewById(R.id.gpsbutton)
        val beepButton: Button = findViewById(R.id.beepbutton)
        val baitButton: Button = findViewById(R.id.baitbutton)
        val waterButton: Button = findViewById(R.id.waterbutton)
        val cctvButton: Button = findViewById(R.id.cctvbutton)
        val fishButton: Button = findViewById(R.id.fishbutton)

        var count = 4

        portButton.setOnClickListener {
            port = portedit.text.toString().toInt()
        }


        fishButton.setOnClickListener {
            val fishcommand = "fish"
            FishTask().execute(fishcommand)
        }

        cctvButton.setOnClickListener {
            // cctvbutton을 클릭했을 때 CctvActivity를 시작합니다.
            val intent = Intent(this@MainActivity, CctvActivity::class.java)
            startActivity(intent)
        }

        waterButton.setOnClickListener {
            val watercommand = "water"
            WaterTask().execute(watercommand)
        }

        baitButton.setOnClickListener {
            val baitcommand = "shoot"
            SendCommandTask().execute(baitcommand)
        }

        beepButton.setOnClickListener {
            val beepcommand = "sound"
            SendCommandTask().execute(beepcommand)
        }

        ledButton.setOnClickListener {
            // 현재 버튼의 상태를 확인하여 명령을 보냄
            val ledcommand = if (ledButton.text == "led켜기") "on" else "off"
            SendCommandTask().execute(ledcommand)
            if (ledButton.text == "led켜기") {
                ledButton.text = "led끄기"
            } else {
                ledButton.text = "led켜기"
            }
        }

        doorButton.setOnClickListener {
            val doorcommand = if (doorButton.text == "문열기") "open" else "close"
            SendCommandTask().execute(doorcommand)
            if (doorButton.text == "문열기") {
                doorButton.text = "문닫기"
            } else {
                doorButton.text = "문열기"
            }
        }

        gpsButton.setOnClickListener {
            val require = "gps"
            GpsTask().execute(require)
        }

    }

    private inner class SendCommandTask : AsyncTask<String, Void, Void>() {
        override fun doInBackground(vararg params: String?): Void? {
            try {
                val temptext: TextView = findViewById(R.id.Numbertext)
                val socket = Socket(ip, port)
                val outputStream: OutputStream = socket.getOutputStream()
                val command = params[0] ?: ""
                outputStream.write(command.toByteArray())

                // 응답을 받기 위한 부분
                val reader = BufferedReader(InputStreamReader(socket.getInputStream()))
                val response = reader.readLine()

                socket.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return null
        }
    }

    private inner class GpsTask : AsyncTask<String, Void, Void>() {
        override fun doInBackground(vararg params: String?): Void? {
            try {
                val temptext: TextView = findViewById(R.id.Numbertext)
                val socket = Socket(ip, port)
                val outputStream: OutputStream = socket.getOutputStream()
                val command = params[0] ?: ""
                outputStream.write(command.toByteArray())

                // 응답을 받기 위한 부분
                val reader = BufferedReader(InputStreamReader(socket.getInputStream()))
                val response = reader.readLine()
                temptext.text = response

                val split = response.split(",")

                val d1 = split[0].toDouble()
                val d2 = split[1].toDouble()

                val intent = Intent(Intent.ACTION_VIEW, Uri.parse("geo:$d1,$d2"))
                startActivity(intent)

                socket.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return null
        }
    }

    private inner class FishTask : AsyncTask<String, Void, Void>() {
        override fun doInBackground(vararg params: String?): Void? {
            try {
                val fishtext: TextView = findViewById(R.id.Numbertext)
                val socket = Socket(ip, port)
                val outputStream: OutputStream = socket.getOutputStream()
                val command = params[0] ?: ""
                outputStream.write(command.toByteArray())

                // 응답을 받기 위한 부분
                val reader = BufferedReader(InputStreamReader(socket.getInputStream()))
                val response = reader.readLine()
                fishtext.text = response.toString()

                socket.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return null
        }
    }

    private inner class WaterTask : AsyncTask<String, Void, Void>() {
        override fun doInBackground(vararg params: String?): Void? {
            try {
                val watertext: TextView = findViewById(R.id.waterText2)
                val temptext: TextView = findViewById(R.id.Numbertext)
                val socket = Socket(ip, port)
                val outputStream: OutputStream = socket.getOutputStream()
                val command = params[0] ?: ""
                outputStream.write(command.toByteArray())

                // 응답을 받기 위한 부분
                val reader = BufferedReader(InputStreamReader(socket.getInputStream()))
                val response = reader.readLine()
                watertext.text = response.toString()

                socket.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return null
        }
    }
}
