package com.example.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.app.ui.theme.AppTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            AppTheme {
                AppNavigator()
            }
        }
    }
}

@Composable
fun AppNavigator() {
    var currentScreen by remember { mutableStateOf("Home") }
    var selectedSensorName by remember { mutableStateOf<String?>(null) }

    val notificationViewModel: NotificationViewModel = viewModel()

    val sensors by notificationViewModel.sensors.collectAsState()
    val systemStatus by notificationViewModel.systemStatus.collectAsState()

    Scaffold(
        modifier = Modifier.fillMaxSize(),
        topBar = {
            var screenTitle = ""
            var backAction: (() -> Unit)? = null

            when (currentScreen) {
                "Home" -> { screenTitle = "Tổng quan" }
                "Dashboard" -> { screenTitle = "Bảng điều khiển" }
                "Chart" -> { backAction = { currentScreen = "Dashboard" } }
                "Notifications" -> { screenTitle = "Thông báo" }
                "About" -> { screenTitle = "Thông tin" }
            }
            BklogyGlobalHeader(title = screenTitle, onBack = backAction)
        },
        bottomBar = {
            if (currentScreen != "Chart") {
                AppFooter(currentScreen = currentScreen, onNavigate = { currentScreen = it })
            }
        }
    ) { innerPadding ->
        Box(modifier = Modifier.padding(innerPadding).fillMaxSize().background(Color(0xFF121212))) {
            when (currentScreen) {
                "Home" -> HomeScreen(systemStatus = systemStatus, viewModel = notificationViewModel)
                "Dashboard" -> SensorGridScreen(sensors = sensors)
                "Notifications" -> NotificationScreen(viewModel = notificationViewModel)
                "About" -> AboutUsScreen()
                "Chart" -> {
                    val activeSensor = sensors.find { it.name == selectedSensorName }
                    if (activeSensor != null) {
                        SensorChartScreen(sensor = activeSensor)
                    }
                }
            }
        }
    }
}