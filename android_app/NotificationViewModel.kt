package com.example.app

import android.app.Application
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.compose.ui.graphics.Color
import androidx.core.app.NotificationCompat
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import org.eclipse.paho.client.mqttv3.*
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence
import org.json.JSONArray
import org.json.JSONObject
import java.net.URL
import java.text.SimpleDateFormat
import java.util.*

class NotificationViewModel(application: Application) : AndroidViewModel(application) {

    private val _notifications = MutableStateFlow<List<NotificationData>>(emptyList())
    val notifications: StateFlow<List<NotificationData>> = _notifications.asStateFlow()

    private val _sensors = MutableStateFlow<List<SensorData>>(emptyList())
    val sensors: StateFlow<List<SensorData>> = _sensors.asStateFlow()

    private val _systemStatus = MutableStateFlow(SystemStatus())
    val systemStatus: StateFlow<SystemStatus> = _systemStatus.asStateFlow()

    private val _selectedIds = MutableStateFlow<Set<String>>(emptySet())
    val selectedIds: StateFlow<Set<String>> = _selectedIds.asStateFlow()

    private val _isSelectionMode = MutableStateFlow(false)
    val isSelectionMode: StateFlow<Boolean> = _isSelectionMode.asStateFlow()

    private val prefs = application.getSharedPreferences("DataloggerPrefs", Context.MODE_PRIVATE)
    private val gson = Gson()
    private var lastSyncTimeMs: Long = prefs.getLong("LAST_SYNC_TIME", 0L)

    private var mqttClient: MqttClient? = null

    // Định nghĩa ID cho kênh thông báo đẩy
    private val CHANNEL_ID = "datalogger_alerts_channel"

    init {
        createNotificationChannel() // Tạo kênh thông báo khi khởi tạo ViewModel
        loadFromLocalStorage()
        syncMissedData()
        connectMqttReal()
    }

    // Hàm tạo Kênh thông báo (Bắt buộc từ Android 8.0 trở lên)
    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "Datalogger Alerts"
            val descriptionText = "Kênh hiển thị cảnh báo sự cố khẩn cấp hệ thống trạm đo"
            val importance = NotificationManager.IMPORTANCE_HIGH // Hiện biểu ngữ đẩy lên màn hình
            val channel = NotificationChannel(CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            val notificationManager = getApplication<Application>().getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }

    // Hàm trigger đẩy thông báo lên thanh trạng thái Android
    private fun triggerSystemNotification(title: String, message: String) {
        val context = getApplication<Application>()
        val builder = NotificationCompat.Builder(context, CHANNEL_ID)
            .setSmallIcon(android.R.drawable.stat_notify_error) // Sử dụng tạm icon mặc định hệ thống
            .setContentTitle(title)
            .setContentText(message)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setAutoCancel(true) // Bấm vào tự động xóa thông báo

        val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        // Dùng thời gian hiện tại làm ID để các thông báo không bị ghi đè đè lên nhau
        notificationManager.notify(System.currentTimeMillis().toInt(), builder.build())
    }

    private fun loadFromLocalStorage() {
        val notiJson = prefs.getString("NOTI_HISTORY", null)
        if (notiJson != null) {
            try {
                val type = object : TypeToken<List<NotificationData>>() {}.type
                _notifications.value = gson.fromJson(notiJson, type)
            } catch (e: Exception) { e.printStackTrace() }
        }

        val sensorsJson = prefs.getString("SENSORS_STATE", null)
        if (sensorsJson != null) {
            try {
                val type = object : TypeToken<List<SensorData>>() {}.type
                _sensors.value = gson.fromJson(sensorsJson, type)
            } catch (e: Exception) { e.printStackTrace() }
        }

        val statusJson = prefs.getString("SYSTEM_STATUS_STATE", null)
        if (statusJson != null) {
            try {
                _systemStatus.value = gson.fromJson(statusJson, SystemStatus::class.java)
            } catch (e: Exception) { e.printStackTrace() }
        }
    }

    private fun saveToLocalStorage(list: List<NotificationData>) {
        val limitedList = list.take(100)
        prefs.edit().putString("NOTI_HISTORY", gson.toJson(limitedList)).apply()
        lastSyncTimeMs = System.currentTimeMillis()
        prefs.edit().putLong("LAST_SYNC_TIME", lastSyncTimeMs).apply()
    }

    private fun saveSensorsToStorage(list: List<SensorData>) {
        try {
            prefs.edit().putString("SENSORS_STATE", gson.toJson(list)).apply()
        } catch (e: Exception) { e.printStackTrace() }
    }

    private fun saveSystemStatusToStorage(status: SystemStatus) {
        try {
            prefs.edit().putString("SYSTEM_STATUS_STATE", gson.toJson(status)).apply()
        } catch (e: Exception) { e.printStackTrace() }
    }

    private fun syncMissedData() {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                val url = URL("https://datalogger-backend-t3te.onrender.com/api/sync?lastSyncTime=$lastSyncTimeMs")
                val connection = url.openConnection() as java.net.HttpURLConnection
                connection.connectTimeout = 3000
                if (connection.responseCode == 200) {
                    val responseText = connection.inputStream.bufferedReader().use { it.readText() }
                    val jsonArray = JSONArray(responseText)
                    if (jsonArray.length() > 0) {
                        val missedList = mutableListOf<NotificationData>()
                        val parser = SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'", Locale.getDefault()).apply {
                            timeZone = TimeZone.getTimeZone("UTC")
                        }
                        val formatter = SimpleDateFormat("HH:mm:ss - dd/MM/yyyy", Locale.getDefault())

                        for (i in 0 until jsonArray.length()) {
                            val item = jsonArray.getJSONObject(i)
                            var formattedTime = item.optString("timestamp", "")
                            try { parser.parse(formattedTime)?.let { formattedTime = formatter.format(it) } } catch (e: Exception) {}

                            missedList.add(NotificationData(
                                id = item.optString("_id", System.currentTimeMillis().toString() + i),
                                alertType = item.optString("alert_type", "Sự kiện đồng bộ"),
                                target = item.optString("target", "Hệ thống"),
                                detail = item.optString("detail", ""),
                                timestamp = formattedTime,
                                severity = when(item.optString("severity", "INFO").uppercase()) {
                                    "CRITICAL" -> NotificationSeverity.CRITICAL
                                    "WARNING" -> NotificationSeverity.WARNING
                                    else -> NotificationSeverity.INFO
                                }
                            ))
                        }
                        _notifications.value = missedList + _notifications.value
                        saveToLocalStorage(_notifications.value)
                    }
                }
            } catch (e: Exception) { e.printStackTrace() }
        }
    }

    private fun connectMqttReal() {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                if (mqttClient == null) {
                    mqttClient = MqttClient("tcp://broker.emqx.io:1883", "AndroidApp_" + System.currentTimeMillis(), MemoryPersistence())
                }

                val options = MqttConnectOptions().apply {
                    isCleanSession = true
                    connectionTimeout = 10
                    keepAliveInterval = 20
                    isAutomaticReconnect = true
                }

                mqttClient?.setCallback(object : MqttCallbackExtended {
                    override fun connectComplete(reconnect: Boolean, serverURI: String?) {
                        try {
                            mqttClient?.subscribe(arrayOf("/datalogger/notif", "/datalogger/sensor", "/datalogger/status"))
                        } catch (e: Exception) { e.printStackTrace() }
                    }
                    override fun connectionLost(cause: Throwable?) {
                        executeManualReconnect()
                    }
                    override fun deliveryComplete(token: IMqttDeliveryToken?) {}
                    override fun messageArrived(topic: String?, message: MqttMessage?) {
                        val payload = message?.toString() ?: return
                        when (topic) {
                            "/datalogger/notif" -> parseAndAddNewEvent(payload)
                            "/datalogger/sensor" -> parseSensorArrayPayload(payload)
                            "/datalogger/status" -> parseStatusPayload(payload)
                        }
                    }
                })
                mqttClient?.connect(options)
            } catch (e: Exception) {
                e.printStackTrace()
                executeManualReconnect()
            }
        }
    }

    private fun executeManualReconnect() {
        viewModelScope.launch(Dispatchers.IO) {
            if (mqttClient?.isConnected == false) {
                try {
                    delay(3000)
                    mqttClient?.connect()
                } catch (e: Exception) {
                    e.printStackTrace()
                    executeManualReconnect()
                }
            }
        }
    }

    private fun parseAndAddNewEvent(rawPayload: String) {
        try {
            val json = JSONObject(rawPayload)
            val formatter = SimpleDateFormat("HH:mm:ss - dd/MM/yyyy", Locale.getDefault())

            val alertType = json.optString("alert_type", "Sự kiện")
            val target = json.optString("target", "Module")
            val detail = json.optString("detail", rawPayload)

            val newEvent = NotificationData(
                id = System.currentTimeMillis().toString(),
                alertType = alertType,
                target = target,
                detail = detail,
                timestamp = formatter.format(Date()),
                severity = when(json.optString("severity", "INFO").uppercase()) {
                    "CRITICAL" -> NotificationSeverity.CRITICAL
                    "WARNING" -> NotificationSeverity.WARNING
                    else -> NotificationSeverity.INFO
                }
            )
            _notifications.value = listOf(newEvent) + _notifications.value
            saveToLocalStorage(_notifications.value)

            triggerSystemNotification(
                title = "[$alertType] - $target",
                message = detail
            )

        } catch (e: Exception) { e.printStackTrace() }
    }

    private fun parseSensorArrayPayload(payload: String) {
        try {
            val jsonArray = JSONArray(payload)
            val incomingSize = jsonArray.length()
            val currentSensorsList = _sensors.value
            val updatedList = mutableListOf<SensorData>()

            for (index in 0 until incomingSize) {
                val incomingData = jsonArray.getJSONObject(index)
                val newValue = incomingData.optDouble("value", 0.0)

                val rawStatusInt = incomingData.optInt("statusText", 0)
                val (statusText, statusColor) = when (rawStatusInt) {
                    1 -> Pair("Calibration", Color(0xFF2196F3))
                    2 -> Pair("Bad", Color(0xFFF44336))
                    else -> Pair("Good", Color(0xFF4CAF50))
                }

                val rawTypeInt = incomingData.optInt("type", 2)
                val (typeName, typeColor) = when (rawTypeInt) {
                    1 -> Pair("MODBUS", Color(0xFF4CAF50))
                    2 -> Pair("ANALOG", Color(0xFF2196F3))
                    3 -> Pair("CAPTURE", Color(0xFFFFC107))
                    else -> Pair("--", Color.Gray)
                }

                val hasOldActiveSensor = index < currentSensorsList.size && currentSensorsList[index].totalSamples > 0
                val oldHistory = if (hasOldActiveSensor) currentSensorsList[index].history else emptyList()
                val updatedHistory = (oldHistory + newValue).takeLast(60)
                val oldSamples = if (hasOldActiveSensor) currentSensorsList[index].totalSamples else 0

                updatedList.add(
                    SensorData(
                        name = incomingData.optString("name", String.format("Cảm biến %02d", index + 1)),
                        history = updatedHistory,
                        totalSamples = oldSamples + 1,
                        unit = incomingData.optString("unit", "--"),
                        statusText = statusText,
                        statusColor = statusColor,
                        type = typeName,
                        typeColor = typeColor
                    )
                )
            }
            _sensors.value = updatedList
            saveSensorsToStorage(updatedList)
        } catch (e: Exception) { e.printStackTrace() }
    }

    private fun parseStatusPayload(payload: String) {
        try {
            val json = JSONObject(payload)
            val services = json.optJSONObject("services")
            val updatedStatus = SystemStatus(
                cpuLoad = json.optDouble("cpuLoad", 0.0).toFloat(),
                memory = json.optDouble("memory", 0.0).toFloat(),
                storage = json.optDouble("storage", 0.0).toFloat(),
                uptime = json.optString("uptime", "--"),
                version = json.optString("version", "--"),
                firmware = json.optString("firmware", "--"),
                model = json.optString("model", "--"),
                serial = json.optString("serial", "--"),
                mqttBrokerStatus = services?.optString("mqttBroker", "Inactive") ?: "Inactive",
                ftpServerStatus = services?.optString("ftpServer", "Inactive") ?: "Inactive",
                rs485ModbusStatus = services?.optString("rs485Modbus", "Inactive") ?: "Inactive",
                networkStatus = services?.optString("network", "Inactive") ?: "Inactive",
                sdCardStatus = services?.optString("sdCard", "No insert") ?: "No insert"
            )
            _systemStatus.value = updatedStatus
            saveSystemStatusToStorage(updatedStatus)
        } catch (e: Exception) { e.printStackTrace() }
    }

    fun markAsRead(id: String) {
        val updatedList = _notifications.value.map { if (it.id == id) it.copy(isRead = true) else it }
        _notifications.value = updatedList
        saveToLocalStorage(updatedList)
    }
    fun enterSelectionMode(id: String) { _isSelectionMode.value = true; _selectedIds.value = setOf(id) }
    fun toggleSelection(id: String) {
        val currentSet = _selectedIds.value.toMutableSet()
        if (currentSet.contains(id)) { currentSet.remove(id); if (currentSet.isEmpty()) _isSelectionMode.value = false }
        else currentSet.add(id)
        _selectedIds.value = currentSet
    }
    fun selectAll() { _selectedIds.value = _notifications.value.map { it.id }.toSet() }
    fun clearSelection() { _isSelectionMode.value = false; _selectedIds.value = emptySet() }
    fun deleteSelected() {
        _notifications.value = _notifications.value.filterNot { _selectedIds.value.contains(it.id) }
        saveToLocalStorage(_notifications.value); clearSelection()
    }
}