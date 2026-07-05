package com.example.app

import androidx.compose.ui.graphics.Color

data class SensorData(
    val name: String,
    val history: List<Double>,
    val totalSamples: Int = 0,
    val unit: String,
    val statusText: String,
    val statusColor: Color,
    val type: String,
    val typeColor: Color
) {
    val currentValue: Double get() = history.lastOrNull() ?: 0.0
}

data class NotificationData(
    val id: String,
    val alertType: String,
    val target: String,
    val detail: String,
    val timestamp: String,
    val severity: NotificationSeverity,
    var isRead: Boolean = false
)

enum class NotificationSeverity { INFO, WARNING, CRITICAL }

data class SystemStatus(
    val cpuLoad: Float = 0f,
    val memory: Float = 0f,
    val storage: Float = 0f,
    val uptime: String = "--",
    val version: String = "--",
    val firmware: String = "--",
    val model: String = "--",
    val serial: String = "--",
    val mqttBrokerStatus: String = "Waiting...",
    val ftpServerStatus: String = "Waiting...",
    val rs485ModbusStatus: String = "Waiting...",
    val networkStatus: String = "Waiting...",
    val sdCardStatus: String = "Waiting..."
)