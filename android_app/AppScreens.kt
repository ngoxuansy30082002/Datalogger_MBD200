package com.example.app

import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.text.style.TextAlign

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BklogyGlobalHeader(title: String, onBack: (() -> Unit)?) {
    TopAppBar(
        title = {
            Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                Image(painter = painterResource(id = R.drawable.bklogy_logo), contentDescription = "Bklogy Logo", modifier = Modifier.height(50.dp), contentScale = ContentScale.Fit)
                if (title.isNotEmpty()) {
                    Spacer(modifier = Modifier.weight(1f))
                    Text(text = title, color = Color.White, fontSize = 20.sp, fontWeight = FontWeight.Bold, maxLines = 1, modifier = Modifier.padding(end = 16.dp))
                }
            }
        },
        navigationIcon = {
            if (onBack != null) {
                IconButton(onClick = onBack) { Icon(Icons.Default.ArrowBack, contentDescription = "Back", tint = Color.White) }
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(containerColor = Color(0xFF1E1E1E))
    )
}

@Composable
fun AppFooter(currentScreen: String, onNavigate: (String) -> Unit) {
    NavigationBar(containerColor = Color(0xFF1E1E1E), contentColor = Color.White) {
        NavigationBarItem(selected = currentScreen == "Home", onClick = { onNavigate("Home") }, icon = { Icon(if (currentScreen == "Home") Icons.Filled.Home else Icons.Outlined.Home, contentDescription = "Home") }, colors = NavigationBarItemDefaults.colors(selectedIconColor = Color.White, unselectedIconColor = Color.Gray, indicatorColor = Color.Transparent))
        NavigationBarItem(selected = currentScreen == "Dashboard", onClick = { onNavigate("Dashboard") }, icon = { Icon(if (currentScreen == "Dashboard") Icons.Filled.Dashboard else Icons.Outlined.Dashboard, contentDescription = "Dashboard") }, colors = NavigationBarItemDefaults.colors(selectedIconColor = Color.White, unselectedIconColor = Color.Gray, indicatorColor = Color.Transparent))
        NavigationBarItem(selected = currentScreen == "Notifications", onClick = { onNavigate("Notifications") }, icon = { Icon(if (currentScreen == "Notifications") Icons.Filled.Notifications else Icons.Outlined.Notifications, contentDescription = "Notifications") }, colors = NavigationBarItemDefaults.colors(selectedIconColor = Color.White, unselectedIconColor = Color.Gray, indicatorColor = Color.Transparent))
        NavigationBarItem(selected = currentScreen == "About", onClick = { onNavigate("About") }, icon = { Icon(if (currentScreen == "About") Icons.Filled.Person else Icons.Outlined.Person, contentDescription = "About") }, colors = NavigationBarItemDefaults.colors(selectedIconColor = Color.White, unselectedIconColor = Color.Gray, indicatorColor = Color.Transparent))
    }
}

@Composable
fun HomeScreen(systemStatus: SystemStatus, viewModel: NotificationViewModel) {
    val scrollState = rememberScrollState()
    Column(modifier = Modifier.fillMaxSize().verticalScroll(scrollState).padding(16.dp), verticalArrangement = Arrangement.spacedBy(16.dp)) {
        HeaderBanner()
        SystemInfoCard(status = systemStatus)
        ServiceStatusCard(status = systemStatus)
        Spacer(modifier = Modifier.height(20.dp))
    }
}


@Composable
fun HeaderBanner() {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2879D0)),
        shape = RoundedCornerShape(8.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1.5f)) {
                Text(
                    text = "DataLogger",
                    color = Color.White,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold
                )

                Spacer(modifier = Modifier.height(8.dp))

                Text(
                    text = "Thiết bị truyền nhận dữ liệu quan trắc tự động, xử lý và lưu trữ dữ liệu từ nhiều loại cảm biến công nghiệp. Hỗ trợ Analog, Modbus RS485, Input Capture và nhiều giao thức khác.",
                    modifier = Modifier.fillMaxWidth(),
                    color = Color.White.copy(alpha = 0.9f),
                    fontSize = 12.sp,
                    lineHeight = 18.sp,
                    textAlign = TextAlign.Justify
                )
            }

            Spacer(modifier = Modifier.width(12.dp))

            Image(
                painter = painterResource(id = R.drawable.datalogger_img),
                contentDescription = "MBD-200 Datalogger",
                modifier = Modifier
                    .weight(1f)
                    .height(130.dp),
                contentScale = ContentScale.Fit
            )
        }
    }
}

@Composable
fun SystemInfoCard(status: SystemStatus) {
    Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C)), border = BorderStroke(1.dp, Color.DarkGray)) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(Icons.Default.Info, contentDescription = null, tint = Color(0xFF2196F3), modifier = Modifier.size(20.dp))
                Spacer(Modifier.width(8.dp))
                Text("Thông tin hệ thống", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = Color.White)
            }
            HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp), color = Color.DarkGray)

            // Đã thay thế giá trị tĩnh bằng các trường động nhận từ SystemStatus MQTT
            InfoRow(label = "Phiên bản", value = status.version)
            InfoRow(label = "Firmware", value = status.firmware)
            InfoRow(label = "Model", value = status.model)
            InfoRow(label = "Serial", value = status.serial)

            HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp), color = Color.DarkGray)
            ProgressRow(label = "CPU Load", progress = status.cpuLoad, percentage = "${(status.cpuLoad * 100).toInt()}%", barColor = Color(0xFF2196F3))
            ProgressRow(label = "Memory", progress = status.memory, percentage = "${(status.memory * 100).toInt()}%", barColor = Color(0xFF4CAF50))
            ProgressRow(label = "Storage", progress = status.storage, percentage = "${(status.storage * 100).toInt()}%", barColor = Color(0xFFFFC107))
            Spacer(Modifier.height(8.dp))
            InfoRow(label = "Uptime", value = status.uptime)
        }
    }
}

@Composable
fun ServiceStatusCard(status: SystemStatus) {
    Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C)), border = BorderStroke(1.dp, Color.DarkGray)) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(Icons.Default.CheckCircle, contentDescription = null, tint = Color(0xFF4CAF50), modifier = Modifier.size(20.dp))
                Spacer(Modifier.width(8.dp))
                Text("Trạng thái dịch vụ", fontSize = 16.sp, fontWeight = FontWeight.Bold, color = Color.White)
            }
            HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp), color = Color.DarkGray)
            ServiceRow(icon = Icons.Default.CloudQueue, name = "MQTT Broker", statusText = status.mqttBrokerStatus)
            ServiceRow(icon = Icons.Default.FolderOpen, name = "FTP Server", statusText = status.ftpServerStatus)
            ServiceRow(icon = Icons.Default.SettingsEthernet, name = "RS485 Modbus", statusText = status.rs485ModbusStatus)
            ServiceRow(icon = Icons.Default.Wifi, name = "Network", statusText = status.networkStatus)
            ServiceRow(icon = Icons.Default.SdStorage, name = "SD card", statusText = status.sdCardStatus)
        }
    }
}

@Composable
fun InfoRow(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 6.dp), horizontalArrangement = Arrangement.SpaceBetween) {
        Text(text = label, color = Color.Gray, fontSize = 14.sp)
        Text(text = value, color = Color.White, fontSize = 14.sp, fontWeight = FontWeight.Medium)
    }
}

@Composable
fun ProgressRow(label: String, progress: Float, percentage: String, barColor: Color) {
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 8.dp), verticalAlignment = Alignment.CenterVertically) {
        Text(text = label, color = Color.Gray, fontSize = 14.sp, modifier = Modifier.weight(1f))
        Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.weight(1.5f)) {
            LinearProgressIndicator(progress = { progress }, modifier = Modifier.weight(1f).height(6.dp).clip(RoundedCornerShape(3.dp)), color = barColor, trackColor = Color(0xFF424242))
            Spacer(Modifier.width(12.dp))
            Text(text = percentage, color = Color.White, fontSize = 14.sp, fontWeight = FontWeight.Bold, modifier = Modifier.width(36.dp))
        }
    }
}

@Composable
fun ServiceRow(icon: ImageVector, name: String, statusText: String) {
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 8.dp), verticalAlignment = Alignment.CenterVertically) {
        Icon(imageVector = icon, contentDescription = null, tint = if (statusText == "Waiting...") Color.Gray else Color(0xFF2196F3), modifier = Modifier.size(22.dp))
        Spacer(Modifier.width(12.dp))
        Text(text = name, color = Color.LightGray, fontSize = 14.sp, modifier = Modifier.weight(1f))

        val bgColor = when (statusText) {
            "Waiting..." -> Color.Gray.copy(alpha = 0.15f)
            "Active" -> Color(0xFF4CAF50).copy(alpha = 0.15f)
            else -> Color(0xFFF44336).copy(alpha = 0.15f)
        }
        val textColor = when (statusText) {
            "Waiting..." -> Color.LightGray
            "Active" -> Color(0xFF81C784)
            else -> Color(0xFFFF6B6B)
        }
        Box(modifier = Modifier.background(bgColor, RoundedCornerShape(12.dp)).padding(horizontal = 12.dp, vertical = 4.dp), contentAlignment = Alignment.Center) {
            Text(text = statusText, color = textColor, fontSize = 12.sp, fontWeight = FontWeight.Bold)
        }
    }
}

@Composable
fun NotificationScreen(viewModel: NotificationViewModel) {
    val notifications by viewModel.notifications.collectAsState()
    val selectedIds by viewModel.selectedIds.collectAsState()
    val isSelectionMode by viewModel.isSelectionMode.collectAsState()
    val listState = rememberLazyListState()

    LaunchedEffect(notifications.size) { if (notifications.isNotEmpty()) listState.animateScrollToItem(0) }

    Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
        Row(modifier = Modifier.fillMaxWidth().padding(bottom = 16.dp), verticalAlignment = Alignment.CenterVertically) {
            if (isSelectionMode) {
                IconButton(onClick = { viewModel.clearSelection() }) { Icon(Icons.Default.Close, contentDescription = "Hủy", tint = Color.White) }
                Text(text = "Đã chọn ${selectedIds.size}", color = Color.White, fontSize = 18.sp, fontWeight = FontWeight.Bold, modifier = Modifier.weight(1f).padding(start = 8.dp))
                TextButton(onClick = { viewModel.selectAll() }) { Text("Chọn tất cả", color = Color(0xFF2196F3)) }
                IconButton(onClick = { viewModel.deleteSelected() }) { Icon(Icons.Default.Delete, contentDescription = "Xóa", tint = Color(0xFFF44336)) }
            } else {
                Text(text = "Lịch sử sự kiện", color = Color.White, fontSize = 20.sp, fontWeight = FontWeight.Bold, modifier = Modifier.weight(1f))
            }
        }
        if (notifications.isEmpty()) {
            Text("Chưa có sự kiện nào được ghi nhận.", color = Color.Gray, modifier = Modifier.padding(top = 20.dp))
        } else {
            LazyColumn(state = listState, modifier = Modifier.fillMaxSize(), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                items(notifications, key = { it.id }) { notif ->
                    NotificationCard(
                        notification = notif, isSelected = selectedIds.contains(notif.id), isSelectionMode = isSelectionMode,
                        onLongClick = { viewModel.enterSelectionMode(notif.id) },
                        onClick = { if (isSelectionMode) viewModel.toggleSelection(notif.id) else viewModel.markAsRead(notif.id) }
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalFoundationApi::class)
@Composable
fun NotificationCard(notification: NotificationData, isSelected: Boolean, isSelectionMode: Boolean, onLongClick: () -> Unit, onClick: () -> Unit) {
    val baseBgColor = if (notification.isRead) Color(0xFF1E1E1E) else Color(0xFF2C2C2C)
    val activeBgColor = if (isSelected) Color(0xFF2196F3).copy(alpha = 0.2f) else baseBgColor
    val iconColor = when (notification.severity) {
        NotificationSeverity.CRITICAL -> Color(0xFFF44336)
        NotificationSeverity.WARNING -> Color(0xFFFFC107)
        NotificationSeverity.INFO -> Color(0xFF2196F3)
    }
    val icon = when (notification.severity) {
        NotificationSeverity.CRITICAL -> Icons.Default.ErrorOutline
        NotificationSeverity.WARNING -> Icons.Default.WarningAmber
        NotificationSeverity.INFO -> Icons.Default.Info
    }

    Card(
        modifier = Modifier.fillMaxWidth().combinedClickable(onClick = onClick, onLongClick = { if (!isSelectionMode) onLongClick() }),
        colors = CardDefaults.cardColors(containerColor = activeBgColor),
        border = BorderStroke(1.dp, if (isSelected) Color(0xFF2196F3) else if (notification.isRead) Color.Transparent else Color.DarkGray)
    ) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.Top) {
            if (isSelected) Icon(Icons.Default.CheckCircle, null, tint = Color(0xFF2196F3), modifier = Modifier.size(28.dp))
            else Icon(icon, null, tint = iconColor, modifier = Modifier.size(28.dp))
            Spacer(modifier = Modifier.width(16.dp))
            Column(modifier = Modifier.weight(1f)) {
                Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                    Text(text = notification.alertType, color = if (notification.isRead) Color.Gray else Color.White, fontSize = 15.sp, fontWeight = FontWeight.Bold, modifier = Modifier.weight(1f))
                    Text(text = notification.timestamp, color = Color.Gray, fontSize = 11.sp)
                }
                Spacer(modifier = Modifier.height(6.dp))
                Text(text = "Đối tượng: ${notification.target}", color = Color.LightGray, fontSize = 13.sp)
                Spacer(modifier = Modifier.height(4.dp))
                Text(text = notification.detail, color = if (notification.isRead) Color.DarkGray else Color.LightGray, fontSize = 14.sp)
            }
        }
    }
}

@Composable
fun AboutUsScreen() {
    val scrollState = rememberScrollState()
    Column(modifier = Modifier.fillMaxSize().verticalScroll(scrollState).padding(16.dp), verticalArrangement = Arrangement.spacedBy(16.dp)) {
        Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C)), border = BorderStroke(1.dp, Color.DarkGray)) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(text = "Chúng tôi là BKLOGY LTD", fontSize = 20.sp, fontWeight = FontWeight.Bold, color = Color.White)
                Spacer(modifier = Modifier.height(12.dp))
                Text(text = "Thành lập từ năm 2016, Bklogy bao gồm các thành viên giàu kinh nghiệm trong lĩnh vực R&D và sản xuất điện tử. Hướng tới mục tiêu phát triển sản phẩm chất lượng cao, công ty cung cấp đa dạng giải pháp cho các ngành công nghiệp khác nhau.", color = Color.LightGray, fontSize = 14.sp, lineHeight = 22.sp, textAlign = TextAlign.Justify,
                modifier = Modifier.fillMaxWidth())
            }
        }
        Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C)), border = BorderStroke(1.dp, Color.DarkGray)) {
            Column(modifier = Modifier.fillMaxWidth().padding(16.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                Text(text = "Thông tin liên hệ", fontSize = 20.sp, fontWeight = FontWeight.Bold, color = Color.White)
                Spacer(modifier = Modifier.height(8.dp))
                Text(text = "Quý khách hàng và đối tác cần thêm thông tin chi tiết, hãy liên hệ với chúng tôi qua các thông tin sau", color = Color.Gray, fontSize = 13.sp, textAlign = TextAlign.Center, lineHeight = 18.sp)
                Spacer(modifier = Modifier.height(24.dp))
                ContactItem(icon = Icons.Outlined.Map, title = "Địa chỉ", content = "217 Đường Nguyễn Sinh Sắc,\nQuận Liên Chiểu, Thành phố Đà Nẵng,\nViệt Nam")
                Spacer(modifier = Modifier.height(20.dp))
                ContactItem(icon = Icons.Outlined.Email, title = "Phone & e-mail", content = "+84 (0) 903 592 983\nsales@bklogy.com")
                Spacer(modifier = Modifier.height(20.dp))
                ContactItem(icon = Icons.Outlined.Schedule, title = "Giờ làm việc", content = "Thứ 2 - Thứ 7: 8 AM - 5 PM")
                Spacer(modifier = Modifier.height(8.dp))
            }
        }
        Spacer(modifier = Modifier.height(20.dp))
    }
}

@Composable
fun ContactItem(icon: ImageVector, title: String, content: String) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Icon(imageVector = icon, contentDescription = title, tint = Color(0xFF2196F3), modifier = Modifier.size(36.dp))
        Spacer(modifier = Modifier.height(8.dp))
        Text(text = title, color = Color(0xFF2196F3), fontSize = 16.sp, fontWeight = FontWeight.Medium)
        Spacer(modifier = Modifier.height(4.dp))
        Text(text = content, color = Color.LightGray, fontSize = 14.sp, textAlign = TextAlign.Center, lineHeight = 20.sp)
    }
}

@Composable
fun SensorGridScreen(sensors: List<SensorData>) {
    LazyVerticalGrid(columns = GridCells.Fixed(2), contentPadding = PaddingValues(8.dp), modifier = Modifier.fillMaxSize()) {
        items(sensors) { sensor -> SensorCard(sensor = sensor) }
    }
}

@Composable
fun SensorCard(sensor: SensorData) {
    Card(
        modifier = Modifier.padding(4.dp).fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C)),
        border = BorderStroke(1.dp, Color.DarkGray)
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Text(text = sensor.name, fontSize = 12.sp, color = Color.LightGray, fontWeight = FontWeight.SemiBold)
                Text(text = sensor.type, fontSize = 10.sp, color = sensor.typeColor, modifier = Modifier.background(sensor.typeColor.copy(alpha = 0.2f), RoundedCornerShape(4.dp)).padding(horizontal = 4.dp, vertical = 2.dp))
            }
            Spacer(modifier = Modifier.height(8.dp))
            Row(verticalAlignment = Alignment.Bottom) {
                val displayValue = if (sensor.totalSamples == 0) "--" else sensor.currentValue.toString()
                Text(text = displayValue, fontSize = 28.sp, fontWeight = FontWeight.Bold, color = Color.White)
                Spacer(modifier = Modifier.width(4.dp))
                Text(text = sensor.unit, fontSize = 14.sp, color = Color.Gray, modifier = Modifier.padding(bottom = 4.dp))
            }
            Spacer(modifier = Modifier.height(16.dp))
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    imageVector = if (sensor.statusText == "Lỗi" || sensor.statusText == "Bad") Icons.Default.Warning else if (sensor.totalSamples == 0) Icons.Default.HourglassEmpty else Icons.Default.CheckCircle,
                    contentDescription = null, tint = sensor.statusColor, modifier = Modifier.size(14.dp)
                )
                Spacer(modifier = Modifier.width(4.dp))
                Text(text = sensor.statusText, fontSize = 12.sp, color = sensor.statusColor, fontWeight = FontWeight.Medium)
            }
        }
    }
}

@Composable
fun SensorChartScreen(sensor: SensorData) {
    val scrollState = rememberScrollState()
    LaunchedEffect(sensor.history.size) { scrollState.scrollTo(scrollState.maxValue) }
    Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
        Card(modifier = Modifier.fillMaxWidth().padding(bottom = 16.dp), colors = CardDefaults.cardColors(containerColor = Color(0xFF2C2C2C))) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text("Giá trị hiện tại", color = Color.Gray, fontSize = 20.sp)
                val displayValue = if (sensor.totalSamples == 0) "--" else "${sensor.currentValue} ${sensor.unit}"
                Text(displayValue, fontSize = 36.sp, fontWeight = FontWeight.Bold, color = sensor.typeColor)
            }
        }
        BoxWithConstraints(modifier = Modifier.weight(1f).fillMaxWidth().background(Color(0xFF2C2C2C), RoundedCornerShape(8.dp))) {
            if (sensor.history.isEmpty()) {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Text("Chưa có đồ thị lịch sử", color = Color.Gray)
                }
            } else {
                Canvas(modifier = Modifier.width(maxOf(maxWidth, 40.dp * sensor.history.size)).fillMaxHeight().padding(16.dp).horizontalScroll(scrollState)) {
                    val maxVal = sensor.history.maxOrNull()?.toFloat() ?: 10f
                    val minVal = sensor.history.minOrNull()?.toFloat() ?: 0f
                    val range = if (maxVal == minVal) 1f else (maxVal - minVal)
                    val path = Path()
                    val stepX = size.width / maxOf(1, sensor.history.size - 1)

                    sensor.history.forEachIndexed { index, value ->
                        val x = index * stepX
                        val y = size.height - ((value.toFloat() - minVal) / range) * size.height
                        if (index == 0) path.moveTo(x, y) else path.lineTo(x, y)
                        drawCircle(sensor.typeColor, radius = 5f, center = Offset(x, y))
                    }
                    drawPath(path, color = sensor.typeColor, style = Stroke(5f))
                }
            }
        }
    }
}