import React, { useState, useCallback } from 'react';
import {
  Box, Button, TextField, Typography, LinearProgress, Dialog, DialogTitle,
  DialogContent, DialogActions, IconButton, alpha,
} from '@mui/material';
import Grid from '@mui/material/Grid2';
import {
  CloudUpload as UploadIcon, Close as CloseIcon, InsertDriveFile as FileIcon,
  CheckCircle as SuccessIcon, Error as ErrorIcon,
} from '@mui/icons-material';
import { useDropzone } from 'react-dropzone';
import { firmwareApi } from '../api/firmwareApi';
import { useSnackbar } from 'notistack';
import { FirmwareUploadMeta } from '../types';

interface FirmwareUploadProps {
  open: boolean;
  onClose: () => void;
  onSuccess: () => void;
}

interface UploadFile {
  file: File;
  status: 'pending' | 'uploading' | 'success' | 'error';
  error?: string;
}

const FirmwareUpload: React.FC<FirmwareUploadProps> = ({ open, onClose, onSuccess }) => {
  const { enqueueSnackbar } = useSnackbar();
  const [files, setFiles] = useState<UploadFile[]>([]);
  const [progress, setProgress] = useState(0);
  const [isUploading, setIsUploading] = useState(false);
  const [metadata, setMetadata] = useState<FirmwareUploadMeta>({
    projectName: '', deviceModel: '', hardwareVersion: '', firmwareVersion: '', description: '',
  });

  const onDrop = useCallback((acceptedFiles: File[]) => {
    const hexFiles = acceptedFiles.filter((f) => f.name.toLowerCase().endsWith('.hex'));
    const rejected = acceptedFiles.length - hexFiles.length;
    if (rejected > 0) enqueueSnackbar(`${rejected} file(s) rejected (only .hex)`, { variant: 'warning' });
    setFiles((prev) => [...prev, ...hexFiles.map((file) => ({ file, status: 'pending' as const }))]);
  }, [enqueueSnackbar]);

  const { getRootProps, getInputProps, isDragActive } = useDropzone({
    onDrop, accept: { 'application/octet-stream': ['.hex'] }, multiple: true,
  });

  const removeFile = (index: number) => setFiles((prev) => prev.filter((_, i) => i !== index));

  const handleUpload = async () => {
    if (files.length === 0) { enqueueSnackbar('Please select files', { variant: 'warning' }); return; }
    if (!metadata.projectName || !metadata.deviceModel || !metadata.hardwareVersion || !metadata.firmwareVersion) {
      enqueueSnackbar('Please fill in all required fields', { variant: 'warning' }); return;
    }
    setIsUploading(true); setProgress(0);
    try {
      const result = await firmwareApi.upload(files.map((f) => f.file), metadata, (p) => setProgress(p));
      if (result.success) {
        enqueueSnackbar(result.message || 'Upload successful!', { variant: 'success' });
        setFiles([]); setMetadata({ projectName: '', deviceModel: '', hardwareVersion: '', firmwareVersion: '', description: '' });
        onSuccess(); onClose();
      }
      if (result.errors?.length > 0) result.errors.forEach((err: { file: string; error: string }) => enqueueSnackbar(`${err.file}: ${err.error}`, { variant: 'error' }));
    } catch (error: any) {
      enqueueSnackbar(error.response?.data?.message || 'Upload failed', { variant: 'error' });
    } finally { setIsUploading(false); setProgress(0); }
  };

  const handleClose = () => { if (!isUploading) { setFiles([]); setProgress(0); onClose(); } };
  const formatSize = (bytes: number) => {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / 1048576).toFixed(1)} MB`;
  };

  return (
    <Dialog open={open} onClose={handleClose} maxWidth="md" fullWidth>
      <DialogTitle sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          <UploadIcon sx={{ color: '#6366f1' }} />
          <Typography variant="h6" sx={{ fontWeight: 700 }}>Upload Firmware</Typography>
        </Box>
        <IconButton onClick={handleClose} disabled={isUploading} size="small"><CloseIcon /></IconButton>
      </DialogTitle>
      <DialogContent dividers>
        {/* Drop zone */}
        <Box {...getRootProps()} sx={{
          p: 4, mb: 3, border: `2px dashed ${isDragActive ? '#6366f1' : alpha('#94a3b8', 0.2)}`,
          borderRadius: 3, backgroundColor: isDragActive ? alpha('#6366f1', 0.05) : alpha('#1e293b', 0.3),
          textAlign: 'center', cursor: 'pointer', transition: 'all 0.2s ease',
          '&:hover': { borderColor: alpha('#6366f1', 0.5), backgroundColor: alpha('#6366f1', 0.03) },
        }}>
          <input {...getInputProps()} />
          <UploadIcon sx={{ fontSize: 48, color: '#64748b', mb: 1 }} />
          <Typography variant="body1" sx={{ color: '#94a3b8', mb: 0.5 }}>
            {isDragActive ? 'Drop files here...' : 'Drag & Drop .hex files here'}
          </Typography>
          <Typography variant="caption" sx={{ color: '#64748b' }}>or click to browse</Typography>
        </Box>

        {/* File list */}
        {files.length > 0 && (
          <Box sx={{ mb: 3 }}>
            <Typography variant="subtitle2" sx={{ mb: 1, color: '#94a3b8' }}>Selected Files ({files.length})</Typography>
            {files.map((f, index) => (
              <Box key={index} sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', p: 1.5, mb: 0.5, borderRadius: 2, backgroundColor: alpha('#1e293b', 0.5) }}>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1.5 }}>
                  {f.status === 'success' ? <SuccessIcon sx={{ color: '#10b981', fontSize: 20 }} /> : f.status === 'error' ? <ErrorIcon sx={{ color: '#ef4444', fontSize: 20 }} /> : <FileIcon sx={{ color: '#64748b', fontSize: 20 }} />}
                  <Box>
                    <Typography variant="body2" sx={{ fontWeight: 500 }}>{f.file.name}</Typography>
                    <Typography variant="caption" sx={{ color: '#64748b' }}>{formatSize(f.file.size)}</Typography>
                  </Box>
                </Box>
                {!isUploading && <IconButton size="small" onClick={() => removeFile(index)}><CloseIcon sx={{ fontSize: 16 }} /></IconButton>}
              </Box>
            ))}
          </Box>
        )}

        {/* Metadata form */}
        <Typography variant="subtitle2" sx={{ mb: 2, color: '#94a3b8' }}>Firmware Metadata</Typography>
        <Grid container spacing={2}>
          <Grid size={{ xs: 12, sm: 6 }}>
            <TextField fullWidth label="Project Name" required value={metadata.projectName} onChange={(e) => setMetadata((m) => ({ ...m, projectName: e.target.value }))} placeholder="e.g., ControllerV5" size="small" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6 }}>
            <TextField fullWidth label="Device Model" required value={metadata.deviceModel} onChange={(e) => setMetadata((m) => ({ ...m, deviceModel: e.target.value }))} placeholder="e.g., Relay64x2" size="small" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6 }}>
            <TextField fullWidth label="Hardware Version" required value={metadata.hardwareVersion} onChange={(e) => setMetadata((m) => ({ ...m, hardwareVersion: e.target.value }))} placeholder="e.g., 1.0" size="small" />
          </Grid>
          <Grid size={{ xs: 12, sm: 6 }}>
            <TextField fullWidth label="Firmware Version" required value={metadata.firmwareVersion} onChange={(e) => setMetadata((m) => ({ ...m, firmwareVersion: e.target.value }))} placeholder="e.g., 5.1.0" size="small" />
          </Grid>
          <Grid size={{ xs: 12 }}>
            <TextField fullWidth label="Description / Release Notes" multiline rows={3} value={metadata.description} onChange={(e) => setMetadata((m) => ({ ...m, description: e.target.value }))} placeholder="Describe changes..." size="small" />
          </Grid>
        </Grid>

        {isUploading && (
          <Box sx={{ mt: 3 }}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
              <Typography variant="caption" sx={{ color: '#94a3b8' }}>Uploading...</Typography>
              <Typography variant="caption" sx={{ color: '#818cf8', fontWeight: 600 }}>{progress}%</Typography>
            </Box>
            <LinearProgress variant="determinate" value={progress} sx={{ height: 6 }} />
          </Box>
        )}
      </DialogContent>
      <DialogActions sx={{ p: 2 }}>
        <Button onClick={handleClose} disabled={isUploading} variant="outlined">Cancel</Button>
        <Button onClick={handleUpload} disabled={isUploading || files.length === 0} variant="contained" startIcon={<UploadIcon />}>
          {isUploading ? 'Uploading...' : `Upload ${files.length} File${files.length !== 1 ? 's' : ''}`}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default FirmwareUpload;
