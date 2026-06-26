import React, { useEffect, useState, useCallback } from 'react';
import {
  Box, Typography, Paper, Table, TableBody, TableCell, TableContainer,
  TableHead, TableRow, TablePagination, Chip, alpha,
} from '@mui/material';
import { FiberManualRecord as Dot } from '@mui/icons-material';
import { deviceApi } from '../api/deviceApi';
import { Device } from '../types';
import { useSocket } from '../contexts/SocketContext';

const DevicesPage: React.FC = () => {
  const [devices, setDevices] = useState<Device[]>([]);
  const [total, setTotal] = useState(0);
  const [page, setPage] = useState(0);
  const [rowsPerPage, setRowsPerPage] = useState(20);
  const [loading, setLoading] = useState(false);
  const { socket } = useSocket();

  const fetchDevices = useCallback(async () => {
    setLoading(true);
    try {
      const result = await deviceApi.list({ page: page + 1, limit: rowsPerPage });
      if (result.success) { setDevices(result.data); setTotal(result.total); }
    } catch { /* silent */ }
    finally { setLoading(false); }
  }, [page, rowsPerPage]);

  useEffect(() => { fetchDevices(); }, [fetchDevices]);

  useEffect(() => {
    if (!socket) return;
    socket.on('device:update', () => fetchDevices());
    return () => { socket.off('device:update'); };
  }, [socket, fetchDevices]);

  const formatDate = (d: string) => new Date(d).toLocaleString();

  return (
    <Box>
      <Typography variant="h4" sx={{ fontWeight: 800, mb: 3, color: '#f1f5f9' }}>Devices</Typography>
      <TableContainer component={Paper} sx={{ backgroundColor: alpha('#111827', 0.6) }}>
        <Table>
          <TableHead>
            <TableRow>
              <TableCell>Device ID</TableCell>
              <TableCell>Model</TableCell>
              <TableCell>HW Version</TableCell>
              <TableCell>Firmware</TableCell>
              <TableCell>Last Seen</TableCell>
              <TableCell>Status</TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
            {loading ? (
              <TableRow><TableCell colSpan={6} align="center" sx={{ py: 4, color: '#64748b' }}>Loading...</TableCell></TableRow>
            ) : devices.length === 0 ? (
              <TableRow><TableCell colSpan={6} align="center" sx={{ py: 4, color: '#64748b' }}>No devices registered yet. Devices appear here when they communicate via MQTT.</TableCell></TableRow>
            ) : devices.map((dev) => (
              <TableRow key={dev._id} hover>
                <TableCell><Typography variant="body2" fontWeight={600}>{dev.deviceId}</Typography></TableCell>
                <TableCell>{dev.deviceModel}</TableCell>
                <TableCell>{dev.hardwareVersion}</TableCell>
                <TableCell>
                  <Chip label={`v${dev.firmwareVersion}`} size="small"
                    sx={{ backgroundColor: alpha('#6366f1', 0.15), color: '#818cf8', fontWeight: 600, fontSize: '0.75rem' }} />
                </TableCell>
                <TableCell><Typography variant="caption" color="#94a3b8">{formatDate(dev.lastSeen)}</Typography></TableCell>
                <TableCell>
                  <Chip
                    icon={<Dot sx={{ fontSize: 10, color: dev.online ? '#10b981' : '#64748b' }} />}
                    label={dev.online ? 'Online' : 'Offline'} size="small"
                    sx={{
                      backgroundColor: dev.online ? alpha('#10b981', 0.1) : alpha('#94a3b8', 0.08),
                      color: dev.online ? '#34d399' : '#64748b', fontWeight: 600, fontSize: '0.7rem',
                    }}
                  />
                </TableCell>
              </TableRow>
            ))}
          </TableBody>
        </Table>
        <TablePagination component="div" count={total} page={page} onPageChange={(_, p) => setPage(p)}
          rowsPerPage={rowsPerPage} onRowsPerPageChange={(e) => { setRowsPerPage(parseInt(e.target.value)); setPage(0); }}
          rowsPerPageOptions={[10, 20, 50]} />
      </TableContainer>
    </Box>
  );
};

export default DevicesPage;
