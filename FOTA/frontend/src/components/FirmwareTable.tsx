import React, { useState, useEffect, useCallback } from 'react';
import {
  Box, Typography, TextField, IconButton, Chip, Tooltip, Button, InputAdornment,
  Table, TableBody, TableCell, TableContainer, TableHead, TableRow, Paper,
  TablePagination, Dialog, DialogTitle, DialogContent, DialogActions, alpha,
} from '@mui/material';
import {
  Search, Download, Delete, Star, StarBorder, Notifications, ContentCopy,
} from '@mui/icons-material';
import { firmwareApi } from '../api/firmwareApi';
import { useSnackbar } from 'notistack';
import { useAuth } from '../contexts/AuthContext';
import { Firmware } from '../types';

interface FirmwareTableProps {
  refreshTrigger: number;
}

const FirmwareTable: React.FC<FirmwareTableProps> = ({ refreshTrigger }) => {
  const { enqueueSnackbar } = useSnackbar();
  const { isAdmin } = useAuth();
  const [firmwares, setFirmwares] = useState<Firmware[]>([]);
  const [total, setTotal] = useState(0);
  const [page, setPage] = useState(0);
  const [rowsPerPage, setRowsPerPage] = useState(10);
  const [search, setSearch] = useState('');
  const [loading, setLoading] = useState(false);
  const [deleteDialog, setDeleteDialog] = useState<string | null>(null);

  const fetchData = useCallback(async () => {
    setLoading(true);
    try {
      const result = await firmwareApi.list({
        page: page + 1, limit: rowsPerPage, search: search || undefined,
      });
      if (result.success) {
        setFirmwares(result.data);
        setTotal(result.total);
      }
    } catch { enqueueSnackbar('Failed to load firmware list', { variant: 'error' }); }
    finally { setLoading(false); }
  }, [page, rowsPerPage, search, enqueueSnackbar]);

  useEffect(() => { fetchData(); }, [fetchData, refreshTrigger]);

  const handleDelete = async () => {
    if (!deleteDialog) return;
    try {
      await firmwareApi.delete(deleteDialog);
      enqueueSnackbar('Firmware deleted', { variant: 'success' });
      setDeleteDialog(null);
      fetchData();
    } catch { enqueueSnackbar('Delete failed', { variant: 'error' }); }
  };

  const handleMarkLatest = async (id: string) => {
    try {
      await firmwareApi.markLatest(id);
      enqueueSnackbar('Marked as latest', { variant: 'success' });
      fetchData();
    } catch { enqueueSnackbar('Failed to mark as latest', { variant: 'error' }); }
  };

  const handleNotify = async (fw: Firmware) => {
    try {
      await firmwareApi.notifyUpdate(fw.firmwareVersion);
      enqueueSnackbar('Notification sent', { variant: 'success' });
    } catch { enqueueSnackbar('Notify failed', { variant: 'error' }); }
  };

  const formatSize = (bytes: number) => {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / 1048576).toFixed(1)} MB`;
  };

  const formatDate = (d: string) => new Date(d).toLocaleString();

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 2, flexWrap: 'wrap', gap: 1 }}>
        <TextField
          size="small" placeholder="Search firmware..." value={search}
          onChange={(e) => { setSearch(e.target.value); setPage(0); }}
          sx={{ width: { xs: '100%', sm: 320 } }}
          InputProps={{
            startAdornment: <InputAdornment position="start"><Search sx={{ color: '#64748b' }} /></InputAdornment>,
          }}
        />
      </Box>

      <TableContainer component={Paper} sx={{ backgroundColor: alpha('#111827', 0.6) }}>
        <Table size="small">
          <TableHead>
            <TableRow>
              <TableCell>Project</TableCell><TableCell>Model</TableCell>
              <TableCell>HW</TableCell><TableCell>FW Version</TableCell>
              <TableCell>Upload Time</TableCell><TableCell>Size</TableCell>
              <TableCell>Downloads</TableCell><TableCell>Status</TableCell>
              <TableCell align="right">Actions</TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
            {loading ? (
              <TableRow><TableCell colSpan={9} align="center" sx={{ py: 4, color: '#64748b' }}>Loading...</TableCell></TableRow>
            ) : firmwares.length === 0 ? (
              <TableRow><TableCell colSpan={9} align="center" sx={{ py: 4, color: '#64748b' }}>No firmware found</TableCell></TableRow>
            ) : (
              firmwares.map((fw) => (
                <TableRow key={fw._id} hover sx={{ '&:hover': { backgroundColor: alpha('#6366f1', 0.03) } }}>
                  <TableCell><Typography variant="body2" fontWeight={500}>{fw.projectName}</Typography></TableCell>
                  <TableCell>{fw.deviceModel}</TableCell>
                  <TableCell>{fw.hardwareVersion}</TableCell>
                  <TableCell>
                    <Chip label={`v${fw.firmwareVersion}`} size="small"
                      sx={{ backgroundColor: alpha('#6366f1', 0.15), color: '#818cf8', fontWeight: 600, fontSize: '0.75rem' }} />
                  </TableCell>
                  <TableCell><Typography variant="caption" color="#94a3b8">{formatDate(fw.uploadTime)}</Typography></TableCell>
                  <TableCell>{formatSize(fw.fileSize)}</TableCell>
                  <TableCell>{fw.downloadCount}</TableCell>
                  <TableCell>
                    {fw.isLatest ? (
                      <Chip label="Latest" size="small" sx={{ backgroundColor: alpha('#10b981', 0.15), color: '#34d399', fontWeight: 600, fontSize: '0.7rem' }} />
                    ) : (
                      <Chip label="Archived" size="small" sx={{ backgroundColor: alpha('#94a3b8', 0.1), color: '#64748b', fontSize: '0.7rem' }} />
                    )}
                  </TableCell>
                  <TableCell align="right">
                    <Box sx={{ display: 'flex', justifyContent: 'flex-end', gap: 0.5 }}>
                      <Tooltip title="Download"><IconButton size="small" onClick={() => firmwareApi.download(fw._id)}><Download sx={{ fontSize: 18 }} /></IconButton></Tooltip>
                      <Tooltip title="Copy SHA256"><IconButton size="small" onClick={() => { navigator.clipboard.writeText(fw.fileHash); enqueueSnackbar('Hash copied', { variant: 'info' }); }}><ContentCopy sx={{ fontSize: 18 }} /></IconButton></Tooltip>
                      {isAdmin && (
                        <>
                          <Tooltip title={fw.isLatest ? 'Is Latest' : 'Mark as Latest'}>
                            <IconButton size="small" onClick={() => handleMarkLatest(fw._id)}>
                              {fw.isLatest ? <Star sx={{ fontSize: 18, color: '#f59e0b' }} /> : <StarBorder sx={{ fontSize: 18 }} />}
                            </IconButton>
                          </Tooltip>
                          <Tooltip title="Notify Update"><IconButton size="small" onClick={() => handleNotify(fw)}><Notifications sx={{ fontSize: 18 }} /></IconButton></Tooltip>
                          <Tooltip title="Delete"><IconButton size="small" onClick={() => setDeleteDialog(fw._id)}><Delete sx={{ fontSize: 18, color: '#ef4444' }} /></IconButton></Tooltip>
                        </>
                      )}
                    </Box>
                  </TableCell>
                </TableRow>
              ))
            )}
          </TableBody>
        </Table>
        <TablePagination component="div" count={total} page={page} onPageChange={(_, p) => setPage(p)} rowsPerPage={rowsPerPage} onRowsPerPageChange={(e) => { setRowsPerPage(parseInt(e.target.value)); setPage(0); }} rowsPerPageOptions={[5, 10, 20, 50]} />
      </TableContainer>

      <Dialog open={!!deleteDialog} onClose={() => setDeleteDialog(null)}>
        <DialogTitle>Confirm Delete</DialogTitle>
        <DialogContent><Typography>Are you sure you want to delete this firmware? This action cannot be undone.</Typography></DialogContent>
        <DialogActions>
          <Button onClick={() => setDeleteDialog(null)}>Cancel</Button>
          <Button onClick={handleDelete} color="error" variant="contained">Delete</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default FirmwareTable;
