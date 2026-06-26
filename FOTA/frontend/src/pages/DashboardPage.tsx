import React, { useEffect, useState, useCallback } from 'react';
import {
  Box, Typography, Paper, Table, TableBody, TableCell,
  TableContainer, TableHead, TableRow, Chip, alpha,
  List, ListItem, ListItemText, Divider, CircularProgress
} from '@mui/material';
import Grid from '@mui/material/Grid2';
import {
  Memory as FirmwareIcon, CloudDownload as DownloadIcon,
  DevicesOther as DeviceIcon, SettingsInputAntenna as MqttIcon
} from '@mui/icons-material';
import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import StatsCard from '../components/StatsCard';
import { dashboardApi } from '../api/dashboardApi';
import { useSocket } from '../contexts/SocketContext';
import { DashboardSummary, WeeklyActivity, Firmware, MqttMessage } from '../types';

const DashboardPage: React.FC = () => {
  const [summary, setSummary] = useState<DashboardSummary | null>(null);
  const [weeklyActivity, setWeeklyActivity] = useState<WeeklyActivity[]>([]);
  const [recentFirmware, setRecentFirmware] = useState<Firmware[]>([]);
  const [mqttLogs, setMqttLogs] = useState<MqttMessage[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  
  const { socket } = useSocket();

  const fetchData = useCallback(async () => {
    try {
      const [summaryRes, activityRes, firmwareRes, mqttRes] = await Promise.all([
        dashboardApi.getSummary(),
        dashboardApi.getWeeklyActivity(),
        dashboardApi.getRecentFirmware(),
        dashboardApi.getMqttLogs(),
      ]);

      if (summaryRes.success) setSummary(summaryRes.data);
      if (activityRes.success) setWeeklyActivity(activityRes.data.reverse()); // Ensure chronological order
      if (firmwareRes.success) setRecentFirmware(firmwareRes.data);
      if (mqttRes.success) setMqttLogs(mqttRes.data);
      
      setError(null);
    } catch (err) {
      console.error('Failed to load dashboard data', err);
      setError('Failed to load dashboard data');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchData();

    // Auto-refresh weekly activity every 60 seconds
    const interval = setInterval(() => {
      fetchData();
    }, 60000);

    return () => clearInterval(interval);
  }, [fetchData]);

  useEffect(() => {
    if (!socket) return;
    
    const handleMqttMessage = (msg: MqttMessage) => {
      setMqttLogs((prev) => [msg, ...prev].slice(0, 20));
      setSummary((prev) => prev ? { ...prev, mqttMessagesToday: prev.mqttMessagesToday + 1 } : prev);
    };

    const handleUpdate = () => fetchData();

    socket.on('mqtt:message', handleMqttMessage);
    socket.on('firmware:uploaded', handleUpdate);
    socket.on('firmware:deleted', handleUpdate);
    socket.on('device:status', handleUpdate);

    return () => {
      socket.off('mqtt:message', handleMqttMessage);
      socket.off('firmware:uploaded', handleUpdate);
      socket.off('firmware:deleted', handleUpdate);
      socket.off('device:status', handleUpdate);
    };
  }, [socket, fetchData]);

  const formatDate = (d: string) => new Date(d).toLocaleString();
  const formatShortDate = (d: string) => new Date(d).toLocaleDateString('en', { month: 'short', day: 'numeric' });
  const formatSize = (b: number) => {
    if (b < 1024) return `${b} B`;
    if (b < 1048576) return `${(b / 1024).toFixed(1)} KB`;
    return `${(b / 1048576).toFixed(1)} MB`;
  };

  if (loading) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
        <CircularProgress />
      </Box>
    );
  }

  if (error) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography color="error">{error}</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Typography variant="h4" sx={{ fontWeight: 800, mb: 3, color: '#f1f5f9' }}>
        Dashboard
      </Typography>

      {/* Stats Cards */}
      {summary && (
        <Grid container spacing={3} sx={{ mb: 4 }}>
          <Grid size={{ xs: 12, sm: 6, md: 3 }}>
            <StatsCard title="Total Firmware" value={summary.totalFirmware} icon={<FirmwareIcon />} color="#6366f1" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6, md: 3 }}>
            <StatsCard title="Total Downloads" value={summary.totalDownloads} icon={<DownloadIcon />} color="#06b6d4" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6, md: 3 }}>
            <StatsCard title="Online Devices" value={summary.onlineDevices} icon={<DeviceIcon />} color="#10b981" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6, md: 3 }}>
            <StatsCard title="MQTT Msgs Today" value={summary.mqttMessagesToday} icon={<MqttIcon />} color="#f59e0b" />
          </Grid>
        </Grid>
      )}

      <Grid container spacing={3}>
        {/* Activity Chart */}
        <Grid size={{ xs: 12, lg: 8 }}>
          <Paper sx={{ p: 3, height: 400, display: 'flex', flexDirection: 'column' }}>
            <Typography variant="h6" sx={{ fontWeight: 700, mb: 2, fontSize: '1rem' }}>
              Firmware Activity (Last 7 Days)
            </Typography>
            <Box sx={{ flexGrow: 1 }}>
              <ResponsiveContainer width="100%" height="100%">
                <AreaChart data={weeklyActivity.map(d => ({ ...d, formattedDate: formatShortDate(d.date) }))}>
                  <defs>
                    <linearGradient id="colorDownloads" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="5%" stopColor="#06b6d4" stopOpacity={0.3} />
                      <stop offset="95%" stopColor="#06b6d4" stopOpacity={0} />
                    </linearGradient>

                    <linearGradient id="colorUploads" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="5%" stopColor="#6366f1" stopOpacity={0.3} />
                      <stop offset="95%" stopColor="#6366f1" stopOpacity={0} />
                    </linearGradient>
                  </defs>
                  <CartesianGrid strokeDasharray="3 3" stroke={alpha('#94a3b8', 0.08)} />
                  <XAxis dataKey="formattedDate" tick={{ fill: '#64748b', fontSize: 12 }} axisLine={false} tickLine={false} />
                  <YAxis tick={{ fill: '#64748b', fontSize: 12 }} axisLine={false} tickLine={false} />
                  <Tooltip
                    contentStyle={{
                      backgroundColor: '#1e293b', border: `1px solid ${alpha('#94a3b8', 0.1)}`,
                      borderRadius: 8, color: '#f1f5f9',
                    }}
                  />
                  <Area type="monotone" name="Downloads" dataKey="downloads" stroke="#06b6d4" fill="url(#colorDownloads)" strokeWidth={2} />
                  <Area type="monotone" name="Uploads" dataKey="uploads" stroke="#6366f1" fill="url(#colorUploads)" strokeWidth={2} />
                </AreaChart>
              </ResponsiveContainer>
            </Box>
          </Paper>
        </Grid>

        {/* MQTT Live Monitor */}
        <Grid size={{ xs: 12, lg: 4 }}>
          <Paper sx={{ p: 3, height: 400, display: 'flex', flexDirection: 'column' }}>
            <Typography variant="h6" sx={{ fontWeight: 700, mb: 2, fontSize: '1rem', display: 'flex', alignItems: 'center' }}>
              <MqttIcon sx={{ mr: 1, color: '#f59e0b' }} fontSize="small" /> MQTT Live Monitor
            </Typography>
            <Box sx={{ flexGrow: 1, overflow: 'auto' }}>
              <List dense disablePadding>
                {mqttLogs.length === 0 ? (
                  <ListItem>
                    <ListItemText primary={<Typography color="textSecondary" variant="body2">No recent messages</Typography>} />
                  </ListItem>
                ) : (
                  mqttLogs.map((log, idx) => (
                    <React.Fragment key={idx}>
                      <ListItem alignItems="flex-start" sx={{ px: 0, py: 1 }}>
                        <ListItemText
                          primary={<Typography variant="body2" sx={{ color: '#38bdf8', fontWeight: 500, wordBreak: 'break-all' }}>{log.topic}</Typography>}
                          secondary={
                            <React.Fragment>
                              <Typography component="span" variant="caption" sx={{ color: '#94a3b8', display: 'block', mt: 0.5 }}>
                                {new Date(log.timestamp).toLocaleTimeString()}
                              </Typography>
                              <Typography component="span" variant="caption" sx={{ color: '#cbd5e1', display: 'block', mt: 0.5, wordBreak: 'break-all' }}>
                                {log.payload.length > 100 ? `${log.payload.substring(0, 100)}...` : log.payload}
                              </Typography>
                            </React.Fragment>
                          }
                        />
                      </ListItem>
                      {idx < mqttLogs.length - 1 && <Divider component="li" sx={{ borderColor: alpha('#94a3b8', 0.1) }} />}
                    </React.Fragment>
                  ))
                )}
              </List>
            </Box>
          </Paper>
        </Grid>

        {/* Recent Uploads */}
        <Grid size={{ xs: 12 }}>
          <Paper sx={{ p: 3, overflow: 'auto' }}>
            <Typography variant="h6" sx={{ fontWeight: 700, mb: 2, fontSize: '1rem' }}>
              Recent Firmware Uploads
            </Typography>
            <TableContainer>
              <Table size="small">
                <TableHead>
                  <TableRow>
                    <TableCell>Project</TableCell>
                    <TableCell>Device Model</TableCell>
                    <TableCell>Version</TableCell>
                    <TableCell>Size</TableCell>
                    <TableCell>Time</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {recentFirmware.length === 0 ? (
                    <TableRow>
                      <TableCell colSpan={5} align="center" sx={{ py: 4, color: '#64748b' }}>
                        No uploads yet
                      </TableCell>
                    </TableRow>
                  ) : (
                    recentFirmware.map((fw: Firmware) => (
                      <TableRow key={fw._id}>
                        <TableCell>
                           <Typography variant="body2">{fw.projectName}</Typography>
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2" fontWeight={500}>{fw.deviceModel}</Typography>
                        </TableCell>
                        <TableCell>
                          <Chip label={`v${fw.firmwareVersion}`} size="small"
                            sx={{ backgroundColor: alpha('#6366f1', 0.15), color: '#818cf8', fontWeight: 600, fontSize: '0.7rem' }} />
                        </TableCell>
                        <TableCell><Typography variant="caption" color="#94a3b8">{formatSize(fw.fileSize)}</Typography></TableCell>
                        <TableCell><Typography variant="caption" color="#94a3b8">{formatDate(fw.uploadTime)}</Typography></TableCell>
                      </TableRow>
                    ))
                  )}
                </TableBody>
              </Table>
            </TableContainer>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default DashboardPage;
