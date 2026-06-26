import React, { useState, useRef } from 'react';
import {
  Box, Typography, Paper, TextField, InputAdornment, Chip, Tooltip, Button, alpha,
} from '@mui/material';
import { FilterList, DeleteSweep, FiberManualRecord as Dot } from '@mui/icons-material';
import { useSocket } from '../contexts/SocketContext';

const MqttMonitorPage: React.FC = () => {
  const { mqttMessages, isConnected, clearMessages } = useSocket();
  const [filter, setFilter] = useState('');
  const listRef = useRef<HTMLDivElement>(null);

  const filtered = filter
    ? mqttMessages.filter((m) =>
        m.topic.toLowerCase().includes(filter.toLowerCase()) ||
        m.payload.toLowerCase().includes(filter.toLowerCase()))
    : mqttMessages;

  const formatPayload = (p: string) => { try { return JSON.stringify(JSON.parse(p), null, 2); } catch { return p; } };
  const formatTime = (ts: string) => new Date(ts).toLocaleTimeString('en', { hour12: false, hour: '2-digit', minute: '2-digit', second: '2-digit' });
  const getTopicColor = (t: string) => {
    if (t.includes('firmware')) return '#6366f1';
    if (t.includes('status')) return '#10b981';
    if (t.includes('heartbeat')) return '#f59e0b';
    return '#06b6d4';
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3, flexWrap: 'wrap', gap: 2 }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
          <Typography variant="h4" sx={{ fontWeight: 800, color: '#f1f5f9' }}>MQTT Monitor</Typography>
          <Chip icon={<Dot sx={{ fontSize: 10, color: isConnected ? '#10b981' : '#ef4444' }} />}
            label={isConnected ? 'Connected' : 'Disconnected'} size="small"
            sx={{ backgroundColor: isConnected ? alpha('#10b981', 0.1) : alpha('#ef4444', 0.1), color: isConnected ? '#34d399' : '#f87171', fontWeight: 600 }} />
        </Box>
        <Box sx={{ display: 'flex', gap: 1 }}>
          <TextField size="small" placeholder="Filter..." value={filter} onChange={(e) => setFilter(e.target.value)} sx={{ width: 260 }}
            InputProps={{ startAdornment: <InputAdornment position="start"><FilterList sx={{ color: '#64748b', fontSize: 20 }} /></InputAdornment> }} />
          <Button variant="outlined" size="small" onClick={clearMessages} startIcon={<DeleteSweep />}>Clear</Button>
        </Box>
      </Box>
      <Paper ref={listRef} sx={{ height: 'calc(100vh - 220px)', overflow: 'auto', p: 0, backgroundColor: alpha('#0a0e1a', 0.6) }}>
        {filtered.length === 0 ? (
          <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'center', height: '100%' }}>
            <Typography variant="body2" sx={{ color: '#475569' }}>No MQTT messages yet. Messages appear here in real-time.</Typography>
          </Box>
        ) : filtered.map((msg, i) => (
          <Box key={i} sx={{ p: 2, borderBottom: `1px solid ${alpha('#94a3b8', 0.05)}`, '&:hover': { backgroundColor: alpha('#6366f1', 0.03) } }}>
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 1.5, mb: 0.5 }}>
              <Chip label={msg.topic} size="small" sx={{ backgroundColor: alpha(getTopicColor(msg.topic), 0.1), color: getTopicColor(msg.topic), fontWeight: 600, fontSize: '0.7rem', fontFamily: 'monospace' }} />
              <Typography variant="caption" sx={{ color: '#475569', fontFamily: 'monospace' }}>{formatTime(msg.timestamp)}</Typography>
            </Box>
            <Box sx={{ mt: 0.5, p: 1.5, borderRadius: 1.5, backgroundColor: alpha('#1e293b', 0.4), fontSize: '0.8rem', color: '#cbd5e1', fontFamily: 'monospace', whiteSpace: 'pre-wrap', wordBreak: 'break-all' }}>
              {formatPayload(msg.payload)}
            </Box>
          </Box>
        ))}
      </Paper>
    </Box>
  );
};

export default MqttMonitorPage;
